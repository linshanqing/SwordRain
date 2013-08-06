#include "swordrain.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "engine.h"
#include "maneuvering.h"

class SRJie: public MasochismSkill{
public:
    SRJie():MasochismSkill("srjie"){

    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        if(!damage.from || !damage.to || damage.from->isKongcheng())
            return;
        if(target->isKongcheng() && !target->getHp())
            return;
        if(target->askForSkillInvoke(objectName())){
            if(target->isKongcheng())
                target->drawCards(target->getHp());
            bool success = target->pindian(damage.from, objectName());
            if(success){
                Room *room = target->getRoom();
                room->broadcastSkillInvoke(objectName());
                DamageStruct dam;
                dam.to = damage.from;
                dam.from = damage.to;
                dam.nature = damage.nature;
                dam.damage = damage.damage;
                room->damage(dam);
            }
        }
        return;
    }
};

SRLengyueCard::SRLengyueCard(){
    mute = true;
}

bool SRLengyueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void SRLengyueCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    if(Sanguosha->getCard(this->getSubcards().first())->isKindOf("Weapon"))
        room->setPlayerFlag(source, "lengyue");
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("srlengyue");
    CardUseStruct use;
    use.from = source;
    use.to = targets;
    use.card = slash;
    room->broadcastSkillInvoke("srlengyue", 1);
    room->useCard(use);
}

class SRLengyueView: public OneCardViewAsSkill{
public:
    SRLengyueView():OneCardViewAsSkill("srlengyue"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@srlengyue";
    }

    virtual bool viewFilter(const Card *to_select) const{
        return true;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        SRLengyueCard *card = new SRLengyueCard;
        card->addSubcard(originalCard);
        card->setSkillName(objectName());
        return card;
    }
};

class SRLengyue: public TriggerSkill{
public:
    SRLengyue():TriggerSkill("srlengyue"){
        events << EventPhaseStart << SlashProceed << TargetConfirmed;
        view_as_skill = new SRLengyueView;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        QList<ServerPlayer *>others = room->getOtherPlayers(player);
        switch(event){
        case EventPhaseStart:{
            if(player->getPhase() != Player::Start)
                return false;
            int myRange = player->getAttackRange(), bigger = 0;
            foreach(ServerPlayer *other, others){
                int hisRange = other->getAttackRange();
                if(hisRange > myRange)
                    bigger ++;
            }
            if(bigger == 0 && !player->isNude()){
                room->askForUseCard(player, "@srlengyue", "@@srlengyue");
            }
            break;
        }
        case SlashProceed:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            const Slash *slash = effect.slash;
            if(slash->isVirtualCard() && slash->getSkillName() == objectName() && player->hasFlag("lengyue")){
                room->setPlayerFlag(player, "-lengyue");
                room->slashResult(effect, NULL);
                return true;
            }
            break;
        }
        case TargetConfirmed:{
            int myRange = player->getAttackRange(), smaller = 0;
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.from == player || !use.to.contains(player))
                return false;
            const Card *card = use.card;
            if(!card->isKindOf("Slash") && !card->isKindOf("Duel") && !card->isKindOf("SavageAssault") && !card->isKindOf("ArcheryAttack"))
                return false;
            foreach(ServerPlayer *other, others){
                int hisRange = other->getAttackRange();
                if(hisRange < myRange)
                    smaller ++;
            }
            if(smaller == 0 && !player->isKongcheng()){
                const Card *card = room->askForCard(player, ".|.|.|hand", "@srlengyuecz", data, Card::MethodNone);
                if(card){
                    room->broadcastSkillInvoke(objectName(), 2);
                    CardsMoveStruct move;
                    move.card_ids << card->getEffectiveId();
                    move.to_place = Player::DiscardPile;
                    CardMoveReason reason(CardMoveReason::S_REASON_RECAST, player->objectName());
                    move.reason = reason;
                    room->moveCards(move, true);
                    player->drawCards(1);
                }
            }
            break;
        }
        }
        return false;
    }
};

class SRYushang: public TriggerSkill{
public:
    SRYushang():TriggerSkill("sryushang"){
        events << CardResponded;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        ResponsedStruct res = data.value<ResponsedStruct>();
        if(int x = res.m_who->getMark("yushang")){
            bool can = false;
            switch(res.m_card->getColor()){
            case Card::Red:{
                if(x == 1)
                    can = true;
                break;
            }
            case Card::Black:{
                if(x == 2)
                    can = true;
                break;
            }
            default:{
                if(res.m_card->getSuit() == Card::NoSuit && x == 3)
                    can = true;
                break;
            }
            }
            if(can && player->askForSkillInvoke(objectName()) && !res.m_who->isNude()){
                room->askForDiscard(res.m_who, objectName(), 1, 1, false, true);
            }
        }
        return false;
    }
};

class SRYushangTri: public TriggerSkill{
public:
    SRYushangTri():TriggerSkill("#sryushang"){
        events << CardUsed << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case CardUsed:{
            CardUseStruct use = data.value<CardUseStruct>();
            room->setPlayerMark(use.from, "yushang",
                                use.card->getColor() == Card::Red?1:(use.card->getColor() == Card::Black?2:3));
            break;
        }
        case CardFinished:{
            foreach(ServerPlayer *who, room->getAlivePlayers()){
                if(who->getMark("yushang"))
                    room->setPlayerMark(who, "yushang", 0);
            }
            break;
        }
        }
        return false;
    }
};

class SRBingyun: public TriggerSkill{
public:
    SRBingyun():TriggerSkill("srbingyun"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if(!death.who->hasSkill(objectName()) || death.damage == NULL || death.damage->from == NULL)
            return false;
        LogMessage log;
        log.from = player;
        log.arg = objectName();
        log.type = "#TriggerSkill";
        room->sendLog(log);
        room->loseMaxHp(death.damage->from);
        return false;
    }
};

class SRWuguGet: public TriggerSkill{
public:
    SRWuguGet():TriggerSkill("#srwugu"){
        events << Damage << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName()) && (target->getMark("jiejin")?(target->getMark("@absord")==0):true);
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature != DamageStruct::Normal){
            room->broadcastSkillInvoke("srwugu", 1);
            player->gainMark("@absord");
        }
        return false;
    }
};

class SRWugu: public TriggerSkill{
public:
    SRWugu():TriggerSkill("srwugu"){
        events << PreHpRecover << HpRecover;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case PreHpRecover:{
            ServerPlayer *source = room->findPlayerBySkillName(objectName());
            RecoverStruct reco = data.value<RecoverStruct>();
            if(source && source->getMark("@absord") && source->askForSkillInvoke(objectName())){
                source->loseMark("@absord");
                reco.recover --;
                player->drawCards(2);
                room->setPlayerMark(source, "wugudr", 1);//mark it
                data = QVariant::fromValue(reco);
            }
            break;
        }
        case HpRecover:{//Draw one card after Recover
            foreach(ServerPlayer *who, room->getAlivePlayers()){
                if(who->getMark("wugudr")){
                    who->setMark("wugudr", 0);
                    who->drawCards(1);
                }
            }
            break;
        }
        }
        return false;
    }
};

class SRJuling: public TriggerSkill{
public:
    SRJuling():TriggerSkill("srjuling"){
        events << AfterDrawNCards;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        QList<ServerPlayer *>julings = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *p, julings){
            if(player != p && p->getHandcardNum() < p->getMaxHp() && p->askForSkillInvoke(objectName())){
                room->broadcastSkillInvoke(objectName());
                p->drawCards(1);
            }
        }
        return false;
    }
};

class SRJiejin: public PhaseChangeSkill{
public:
    SRJiejin():PhaseChangeSkill("srjiejin"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->hasSkill(objectName()) && target->getPhase() == Player::Start &&
                !target->getMark("jiejin") && target->getMark("@absord") >= 2;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        room->broadcastSkillInvoke(objectName());
        target->setMark("jiejin", 1);
        target->gainMark("@waked");
        room->loseMaxHp(target);
        room->acquireSkill(target, "srjuling");
        int x = target->getMark("@absord");
        target->loseAllMarks("@absord");
        target->drawCards(x);
        return false;
    }
};

class SRJingxiang: public TriggerSkill{
public:
    SRJingxiang():TriggerSkill("srjingxiang"){
        events << PostHpReduced << EventPhaseStart << HpChanged << Damaged;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case PostHpReduced:{
            if(player->getHp() <= 0 && player->getMark("@jingxiang") == 0 && player->askForSkillInvoke(objectName())){
                room->broadcastSkillInvoke(objectName(), 1);
                player->gainMark("@jingxiang");
                room->setPlayerProperty(player, "hp", QVariant(0));
                return true;
            }
            if(player->getHp() <= 0 && player->getMark("@jingxiang")){
                room->broadcastSkillInvoke(objectName(), 1);
                room->setPlayerProperty(player, "hp", QVariant(0));
                return true;
            }
            break;
        }
        case Damaged:{
            DamageStruct damage = data.value<DamageStruct>();
            if(player->getMark("@jingxiang") > 0 && player->getHp() < 1 && damage.nature == DamageStruct::Thunder){
                room->broadcastSkillInvoke(objectName(), 2);
                player->gainMark("@jingxiang");
            }
            break;
        }
        case HpChanged:{
            if(player->getHp() > 0 && player->getMark("@jingxiang")){
                room->broadcastSkillInvoke(objectName(), 3);
                player->loseAllMarks("@jingxiang");
            }
            break;
        }
        case EventPhaseStart:{
            switch(player->getPhase()){
            case Player::Start:{
                if(player->getMark("@jingxiang")){
                    room->broadcastSkillInvoke(objectName(), 1);
                    player->gainMark("@jingxiang");
                }
                break;
            }
            case Player::NotActive:{
                if(player->getMark("@jingxiang") >= 3){
                    room->broadcastSkillInvoke(objectName(), 4);
                    room->killPlayer(player);
                }
                break;
            }
            default: break;
            }
        }
        default: break;
        }
        return false;
    }
};

class SRHuanming: public TriggerSkill{
public:
    SRHuanming():TriggerSkill("srhuanming"){
        events << CardFinished << EventPhaseEnd;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case CardFinished:{
            CardUseStruct use = data.value<CardUseStruct>();
            if(!use.card->isVirtualCard() && use.card->isKindOf("Slash")
                    //&& room->getCardPlace(use.card->getEffectiveId()) == Player::PlaceTable
                    && player->getPile("ming").length() < 4){
                room->broadcastSkillInvoke(objectName(), 1);
                player->addToPile("ming", use.card);
            }
            break;
        }
        case EventPhaseEnd:{
            QList<int>mings = player->getPile("ming");
            if(!mings.isEmpty() && player->getPhase() == Player::Finish && player->askForSkillInvoke(objectName())){
                room->fillAG(mings, player);
                int dis = room->askForAG(player, mings, false, objectName());
                player->invoke("clearAG");
                Card *card = Sanguosha->getCard(dis);
                CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, player->objectName());
                room->throwCard(card, reason, NULL);
                switch(card->getSuit()){
                case Card::Spade:{
                    room->broadcastSkillInvoke(objectName(), 2);
                    QList<int>to_backs;
                    foreach(int id, room->getDiscardPile()){
                        Card *to_back = Sanguosha->getCard(id);
                        if(to_back->isKindOf("Slash"))
                            to_backs << id;
                    }
                    if(to_backs.isEmpty())
                        return false;
                    ServerPlayer *who = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
                    room->fillAG(to_backs, who);
                    int get = room->askForAG(who, to_backs, false, objectName());
                    who->invoke("clearAG");
                    CardsMoveStruct move;
                    move.card_ids << get;
                    move.to = who;
                    move.to_place = Player::PlaceHand;
                    CardMoveReason reason(CardMoveReason::S_REASON_GOTBACK, who->objectName());
                    move.reason = reason;
                    room->moveCards(move, true);
                    break;
                }
                case Card::Heart:{
                    AmazingGrace *amg = new AmazingGrace(Card::NoSuit, 0);
                    amg->setSkillName(objectName());
                    CardUseStruct use;
                    use.from = player;
                    use.card = amg;
                    room->broadcastSkillInvoke(objectName(), 3);
                    room->useCard(use);
                    break;
                }
                case Card::Club:{
                    room->broadcastSkillInvoke(objectName(), 4);
                    player->drawCards(1);
                    break;
                }
                case Card::Diamond:{
                    room->broadcastSkillInvoke(objectName(), 5);
                    QList<ServerPlayer *>to_select;
                    foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                        if(!p->isKongcheng())
                            to_select << p;
                    }
                    if(to_select.isEmpty())
                        return false;
                    ServerPlayer *who = room->askForPlayerChosen(player, to_select, objectName());
                    room->askForDiscard(who, objectName(), 1, 1);
                    break;
                }
                default: break;
                }
            }
        }
        default: break;
        }
        return false;
    }
};

SRYuyanCard::SRYuyanCard(){

}

bool SRYuyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return !to_select->isKongcheng() && targets.isEmpty() && to_select != Self;
}

void SRYuyanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    Card::Suit suit = room->askForSuit(source, "sryuyan");
    QString cardType = room->askForChoice(source, "sryuyan", "basic+trick+equip", QVariant());
    ServerPlayer *dest = targets.first();
    int id = room->askForCardChosen(source, dest, "h", "sryuyan");
    Card *cd = Sanguosha->getCard(id);
    CardMoveReason reason(CardMoveReason::S_REASON_ROB, source->objectName());
    room->moveCardTo(cd, source, Player::PlaceHand, reason, true);
    if(cd->getSuit() == suit && cd->getType() == cardType)
        return;
    room->setPlayerFlag(source, "yuyanfail");
    room->askForDiscard(source, "sryuyan", 1, 1);
    return;
}

class SRYuyan: public ZeroCardViewAsSkill{//The skill should be "指定其他一名有手牌角色，然后说出一种花色和卡的类型"
public:
    SRYuyan():ZeroCardViewAsSkill("sryuyan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasFlag("yuyanfail");
    }

    virtual const Card *viewAs() const{
        return new SRYuyanCard;
    }
};

class SRMeixi: public TriggerSkill{
public:
    SRMeixi():TriggerSkill("srmeixi"){
        events << Dying;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        if(dying.who != player || player->isKongcheng())
            return false;
        if(room->askForCard(player, "BasicCard,TrickCard|.|.|.", "@srmeixi")){
            room->broadcastSkillInvoke(objectName());
            CardsMoveStruct move;
            QList<int>ids = room->getNCards(1, false);
            move.to_place = Player::PlaceTable;
            move.card_ids = ids;
            CardMoveReason reason(CardMoveReason::S_REASON_TURNOVER, player->objectName());
            move.reason = reason;
            room->moveCardsAtomic(move, true);
            room->getThread()->delay();
            Card *card = Sanguosha->getCard(ids.first());
            if(card->isRed()){
                RecoverStruct reco;
                reco.who = player;
                reco.card = card;
                reco.recover = 1 - player->getHp();
                room->recover(player, reco);
            }
        }
        return false;
    }
};

class SRHongfu: public TriggerSkill{
public:
    SRHongfu():TriggerSkill("srhongfu"){
        events << TurnStart << EventPhaseStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        Room *room = target->getRoom();
        ServerPlayer *who = room->findPlayerBySkillName(objectName());
        return who != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *from = room->findPlayerBySkillName(objectName());
        switch(event){
        case EventPhaseStart:{
            if(player->getPhase() == Player::Start){
                room->setPlayerMark(player, "HongfuCan", 0);
                foreach(ServerPlayer *p, room->getPlayers()){
                    if(p->getMark("skip") && p->getHp()){
                        p->setAlive(true);
                        room->broadcastProperty(p, "alive");
                        room->setPlayerMark(p, "skip", 0);
                    }
                }
                if(from && from->getMark("HongfuAtten")){
                    ServerPlayer *current = room->getCurrent();
                    if(current->hasSkill(objectName()))
                        current->drawCards(2);
                    QList<Player::Phase>phases;
                    phases << Player::Play;
                    from->play(phases);
                    room->setPlayerMark(from, "HongfuAtten", 0);
                }
            }
            break;
        }
        case TurnStart:{
            if(from && !player->hasSkill(objectName()) && !from->getMark("HongfuAtten") && player->getLostHp() > 0){
                if(!from->askForSkillInvoke(objectName()))
                    return false;
                room->setPlayerMark(from, "HongfuCan", 1);
                player->addMark("skip");
                player->setAlive(false);
                room->broadcastProperty(player, "alive");
                room->loseHp(from);
                if(from->getHp() < player->getHp())
                    room->setPlayerMark(from, "HongfuAtten", 1);
                if(from->isAlive() && from->hasSkill(objectName())){
                    LogMessage log;
                    log.type = "#HongfuAttention";
                    log.from = from;
                    log.arg = objectName();
                    room->sendLog(log);
                }
            }
            break;
        }
        }
        return false;
    }
};

class SRHongfuClear: public TriggerSkill{
public:
    SRHongfuClear():TriggerSkill("#srhongfu-clear"){
        events << EventLoseSkill << Death;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        bool tri = false;
        switch(event){
        case EventLoseSkill:{
            QString skill = data.toString();
            if(skill == "srhongfu")
                tri = true;
            break;
        }
        case Death:{
            DeathStruct death = data.value<DeathStruct>();
            if(death.who->hasSkill(objectName()))
                tri = true;
            break;
        }
        }
        if(!tri) return false;
        foreach(ServerPlayer *p, room->getPlayers()){
            if(p->getMark("skip") && p->getHp() > 0){
                p->setAlive(true);
                room->broadcastProperty(p, "alive");
                if(event == Death)
                    p->throwAllHandCards();
                room->setPlayerMark(p, "skip", 0);
            }
        }
        return false;
    }
};

class SRQushiRecord: public TriggerSkill{
public:
    SRQushiRecord():TriggerSkill("#srqushi-record"){
        events << GameStart << Death << EventLoseSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("srqushi");
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case GameStart:{
            QStringList tag;
            tag << "cancel";
            room->setTag("QushiRecord", QVariant(tag));
            break;
        }
        case Death:{
            DeathStruct death = data.value<DeathStruct>();
            if(!death.who->hasSkill("srqushi")){
                QStringList tag = room->getTag("QushiRecord").toStringList();
                tag << death.who->getGeneralName();
                room->setTag("QushiRecord", QVariant(tag));
            }
            break;
        }
        case EventLoseSkill:{
            if(data.toString() == "srqushi"){
                room->removeTag("QushiRecord");
            }
        }
        }
        return false;
    }
};

class SRQushi: public TriggerSkill{
public:
    SRQushi():TriggerSkill("srqushi"){
        events << Death;
    }

    virtual int getPriority() const{
        return 10;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        if(!death.who->hasSkill(objectName()))
            return false;
        QStringList list = room->getTag("QushiRecord").toStringList();
        if(list.length() == 1)
            return false;
        QString choice = room->askForChoice(player, objectName(), list.join("+"), data);
        if(choice != "cancel"){
            room->revivePlayer(player);
            room->changeHero(player, choice, true, true);
            room->loseMaxHp(player);
        }
        return false;
    }
};

class SRLiangshuang: public TriggerSkill{
public:
    SRLiangshuang():TriggerSkill("srliangshuang"){
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if(change.from == Player::Play && player->askForSkillInvoke(objectName())){
            QList<int> cards = room->getNCards(3);
            CardsMoveStruct move;
            move.card_ids = cards;
            move.to_place = Player::PlaceTable;
            CardMoveReason reason(CardMoveReason::S_REASON_TURNOVER, player->objectName());
            move.reason = reason;
            room->moveCardsAtomic(move, true);
            room->getThread()->delay();
            QList<int>to_get, to_throw;
            foreach(int id, cards){
                Card *card = Sanguosha->getCard(id);
                if(card->getSuit() == Card::Club)
                    to_get << id;
                else
                    to_throw << id;
            }
            CardsMoveStruct get, thr;
            get.card_ids = to_get;
            get.to = player;
            get.to_place = Player::PlaceHand;
            CardMoveReason getreason(CardMoveReason::S_REASON_GOTBACK, player->objectName());
            get.reason = getreason;
            thr.card_ids = to_throw;
            thr.to_place = Player::DrawPile;
            CardMoveReason thrreason(CardMoveReason::S_REASON_PUT, player->objectName());
            thr.reason = thrreason;
            QList<CardsMoveStruct>moves;
            if(!to_get.isEmpty())
                moves.push_back(get);
            if(!to_throw.isEmpty())
                moves.push_back(thr);
            room->moveCardsAtomic(moves, true);
        }
        return false;
    }
};

class SRFazhen: public TriggerSkill{
public:
    SRFazhen():TriggerSkill("srfazhen"){
        events << CardEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->hasSkill("srfazhen") && !target->isKongcheng() && TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.from == player || !effect.card->isKindOf("Slash"))
            return false;
        if(effect.card->isVirtualCard() && (effect.card->subcardsLength() > 1 || effect.card->subcardsLength() == 0))
            return false;
        const Card *card = effect.card;
        const Card *dis = room->askForCard(player, ".", "@fazhen", data);
        if(dis){
            if(dis->getSuit() == Card::Club){
                if(dis->getNumber() + 3 > card->getNumber())
                    return true;
                effect.to->obtainCard(dis);
            }else{
                if(dis->getNumber() > card->getNumber())
                    return true;
                effect.to->obtainCard(dis);
            }
        }
        return false;
    }
};

class SRDoubleWeapon: public TriggerSkill{
public:
    SRDoubleWeapon():TriggerSkill("srdoubleweapon"){
        events << CardsMoveOneTime << BeforeCardsMove;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
        switch(event){
        case CardsMoveOneTime:{
            if(move->from && move->from == player){
                if(move->from_places.contains(Player::PlaceEquip)){
                    foreach(int id, move->card_ids){
                        Card *card = Sanguosha->getCard(id);
                        if(card->isKindOf("Weapon") && card->hasFlag("double")){//this skill has bugs
                            if(player->getWeapon()){
                                player->removePileByName("theOtherWeapon");
                                player->addToPile("theOtherWeapon", id);
                            }else{
                                CardsMoveStruct move;
                                move.card_ids = player->getPile("theOtherWeapon");
                                if(move.card_ids.isEmpty())
                                    return false;
                                move.to = player;
                                move.to_place = Player::PlaceEquip;
                                room->moveCards(move, false);
                            }
                        }
                    }
                }
            }
            break;
        }
        case BeforeCardsMove:{
            if(!move->from || move->from != player)
                break;
            foreach (int id, move->card_ids) {
                Card *card = Sanguosha->getCard(id);
                if(card->isKindOf("Weapon") && room->getCardPlace(id) == Player::PlaceEquip)
                    room->setCardFlag(id, "double");
            }
            break;
        }
        default: break;
        }
        return false;
    }
};

class SRDoubleSlash: public TargetModSkill{
public:
    SRDoubleSlash():TargetModSkill("#srdouble"){

    }

    virtual int getResidueNum(const Player *from, const Card *card) const{
        if(from->hasSkill(objectName()) && !(from->getPile("theOtherWeapon").isEmpty()))
            return 1;
        else
            return 0;
    }

    virtual int getExtraTargetNum(const Player *from, const Card *card) const{
        if(from->hasSkill(objectName()) && !from->getPile("theOtherWeapon").isEmpty()){
            if(from->getWeapon()){
                QString name = from->getWeapon()->objectName();
                if(name == "DoubleSword" || name == "IceSword" || name == "QinggangSword")
                    return 1;
            }
        }
        return 0;
    }
};

class SRWuling: public TriggerSkill{
public:
    SRWuling():TriggerSkill("srwuling"){
        events << EventPhaseProceeding;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(player->getPhase()){
        case Player::Start:{
            if(player->getHandcardNum() <= player->getHp() && player->getPile("theOtherSword").isEmpty()){
                player->setFlags("srwuling");
                int x = qrand()%5 + 1;
                switch(x){
                case 1: room->acquireSkill(player, "lieren"); break;
                case 2: room->acquireSkill(player, "nosxuanfeng"); break;
                case 3: room->acquireSkill(player, "ganlu"); break;
                case 4: room->acquireSkill(player, "srleizhou"); break;
                case 5: room->acquireSkill(player, "srluoyan"); break;
                default: break;
                }
            }
            break;
        }
        case Player::Finish:{
            if(player->hasFlag("srwuling")){
                if(player->hasSkill("lieren"))
                    room->detachSkillFromPlayer(player, "lieren");
                if(player->hasSkill("nosxuanfeng"))
                    room->detachSkillFromPlayer(player, "nosxuanfeng");
                if(player->hasSkill("ganlu"))
                    room->detachSkillFromPlayer(player, "ganlu");
                if(player->hasSkill("srleizhou"))
                    room->detachSkillFromPlayer(player, "srleizhou");
                if(player->hasSkill("srluoyan"))
                    room->detachSkillFromPlayer(player, "srluoyan");
            }
            break;
        }
        default: break;
        }
        return false;
    }
};

class SRLeizhouEffect: public TriggerSkill{
public:
    SRLeizhouEffect():TriggerSkill("#leizhoueffect"){
        events << SlashMissed;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        const Slash *slash = effect.slash;
        if(slash->hasFlag("LeizhouSlash")){
            DamageStruct damage;
            damage.from = effect.to;
            damage.to = effect.from;
            damage.damage = 1;
            damage.nature = DamageStruct::Thunder;
            room->damage(damage);
        }
        return false;
    }
};

SRYidaoCard::SRYidaoCard(){

}

bool SRYidaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getHp() <= Self->getHp();
}

void SRYidaoCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    int maxCard = 0;
    foreach(ServerPlayer *p, room->getAlivePlayers()){
        if(p->getHandcardNum() > maxCard)
            maxCard = p->getHandcardNum();
    }
    QList<ServerPlayer *>dests;
    foreach (ServerPlayer *p, room->getAlivePlayers()) {
        if(p->getHandcardNum() == maxCard)
            dests << p;
    }
    ServerPlayer *dest, *target = targets.first();
    if(dests.length() == 1)
        dest = dests.first();
    else
        dest = room->askForPlayerChosen(target ,dests, "sryidao");
    CardsMoveStruct move;
    move.to = target;
    move.to_place = Player::PlaceHand;
    QList<int>to_get = dest->handCards().mid(0, maxCard/2);
    move.card_ids = to_get;
    CardMoveReason reason(CardMoveReason::S_REASON_GIVE, dest->objectName());
    move.reason = reason;
    room->moveCards(move, false);
    if(move.card_ids.length() > 2)
        source->turnOver();
}

class SRYidao: public ZeroCardViewAsSkill{
public:
    SRYidao():ZeroCardViewAsSkill("sryidao"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("SRYidaoCard");
    }

    virtual const Card *viewAs() const{
        return new SRYidaoCard;
    }
};

class SRQingdeng: public TriggerSkill{
public:
    SRQingdeng():TriggerSkill("srqingdeng"){
        events << EventPhaseChanging;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->isAlive();
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if(change.to != Player::NotActive)
            return false;
        ServerPlayer *source = player->getNext();// This may cause bug with extra turns.
        if(!source->isAlive() && source->hasSkill(objectName())){
            ServerPlayer *dest = room->askForPlayerChosen(source, room->getAlivePlayers(), objectName());
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = source;
            log.arg = objectName();
            room->sendLog(log);
            dest->drawCards(1);
        }
        return false;
    }
};

class SRTanyun: public OneCardViewAsSkill{
public:
    SRTanyun():OneCardViewAsSkill("srtanyun"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasFlag("tanyun");
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isRed();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Snatch *snatch = new Snatch(originalCard->getSuit(), originalCard->getNumber());
        snatch->addSubcard(originalCard);
        snatch->setSkillName(objectName());
        return snatch;
    }
};

class SRTanyunTar: public TargetModSkill{
public:
    SRTanyunTar():TargetModSkill("#srtanyun-tar"){
        pattern = "Snatch";
    }

    virtual int getDistanceLimit(const Player *from, const Card *card) const{
        if(from->hasSkill("srtanyun") && card->getSkillName() == "srtanyun")
            return 1;
        else
            return 0;
    }
};

class SRZhangjian: public TriggerSkill{
public:
    SRZhangjian():TriggerSkill("srzhangjian"){
        events << CardsMoveOneTime;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
        if(player->getPhase() != Player::NotActive || move->to != player)
            return false;
        QList<int>ids = move->card_ids;
        Duel *duel;
        if(ids.length() == 1){
            Card *card = Sanguosha->getCard(ids.first());
            duel = new Duel(card->getSuit(), card->getNumber());
        }else
            duel = new Duel(Card::NoSuit, 0);
        QList<ServerPlayer *>dests;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            if(!player->isProhibited(p, duel))
                dests << p;
        }
        QStringList choicelist;
        choicelist << "discard";
        if(!dests.isEmpty())
            choicelist << "duel";
        else;
        QString choice = room->askForChoice(player, objectName(), choicelist.join("+"), data);
        if(choice == "discard"){
            delete duel;
            CardsMoveStruct move;
            move.card_ids = ids;
            move.to = NULL;
            move.to_place = Player::DiscardPile;
            CardMoveReason reason(CardMoveReason::S_REASON_DISCARD, player->objectName());
            move.reason = reason;
            room->moveCards(move, true);
        }else{
            foreach(int id, ids){
                duel->addSubcard(id);
            }
            CardUseStruct use;
            use.card = duel;
            use.from = player;
            use.to << room->askForPlayerChosen(player, dests, objectName());
            room->useCard(use);
        }
        return false;
    }
};

SRFenwuCard::SRFenwuCard(){

}

bool SRFenwuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return to_select->isAlive() && to_select != Self;
}

bool SRFenwuCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void SRFenwuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *first = targets.first(), *second = targets.last();
    FireSlash *slash1 = new FireSlash(Card::NoSuit, 0), *slash2 = new FireSlash(Card::NoSuit, 0);
    CardUseStruct use1, use2;
    use1.card = slash1;
    use1.from = first;
    use1.to << second;
    use2.from = second;
    use2.card = slash2;
    use2.to << first;
    room->useCard(use1);
    room->useCard(use2);
    if(first && first->isAlive()) first->drawCards(1);
    if(second && second->isAlive()) second->drawCards(2);
}

class SRFenwu: public ZeroCardViewAsSkill{
public:
    SRFenwu():ZeroCardViewAsSkill("srfenwu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("SRFenwuCard");
    }

    virtual const Card *viewAs() const{
        return new SRFenwuCard;
    }
};

class SRHuolin: public ProhibitSkill{
public:
    SRHuolin():ProhibitSkill("srhuolin"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return to->hasSkill(objectName()) && (card->isKindOf("FireSlash") || card->isKindOf("FireAttack"));
    }
};

class SRHuimeng: public TriggerSkill{
public:
    SRHuimeng():TriggerSkill("srhuimeng"){
        frequency = Compulsory;
        events << EventPhaseStart << EventPhaseEnd;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::Play)
            return false;
        switch(event){
        case EventPhaseStart:{
            QList<Player::Place>places;
            QList<ServerPlayer *>players;
            for(int i = 1; i <= 160; i++){
                places[i] = room->getCardPlace(i);
                players[i] = room->getCardOwner(i);
            }
            room->setTag("HuimengPlace", QVariant::fromValue(places));
            room->setTag("HuimengPlayer", QVariant::fromValue(players));
            break;
        }
        case EventPhaseEnd:{
            QList<Player::Place>places;
            QList<ServerPlayer *>players;
            QVariantList places_data = room->getTag("HuimengPlace").toList();
            QVariantList player_data = room->getTag("HuimengPlayer").toList();
            room->removeTag("HuimengPlace");
            room->removeTag("HuimengPlayer");
            for(int i = 1; i <= 160; i++){
                places[i] = places_data[i].value<Player::Place>();
                players[i] = player_data[i].value<ServerPlayer *>();
                room->moveCardTo(Sanguosha->getCard(i), players[i], places[i], true);
            }
            break;
        }
        default: break;
        }
        return false;
    }
};

class SRHuimengBack: public TriggerSkill{
public:
    SRHuimengBack():TriggerSkill("#srhuimeng"){
        frequency = Compulsory;
        events << CardsMoving << BeforeCardsMove;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
        if(move->from != player || !move->from_places.contains(Player::PlaceHand))
            return false;

        switch(event){
        case BeforeCardsMove:{
            int x = 0;
            foreach(int id, move->card_ids){
                Card *card = Sanguosha->getCard(id);
                if((card->isKindOf("DelayedTrick") || card->isKindOf("DefensiveHorse")) && room->getCardPlace(id) == Player::PlaceHand)
                    x ++;
            }
            room->setPlayerMark(player, "huimeng", x);
            break;
        }
        case CardsMoving:{
            if(player->getMark("huimeng"))
                player->drawCards(player->getMark("huimeng"));
            break;
        }
        default: break;
        }
        return false;
    }
};

class SRDielian: public TriggerSkill{
public:
    SRDielian():TriggerSkill("srdielian"){
        events << Dying;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        if(dying.who == player || !player->hasSkill(objectName()))
            return false;

        if(player->askForSkillInvoke(objectName(), data)){
            int hp = player->getHp();
            room->killPlayer(player);
            int maxhp = dying.who->getMaxHp();
            room->setPlayerProperty(dying.who, "maxhp", QVariant(maxhp+1));
            RecoverStruct recover;
            recover.who = NULL;
            recover.recover = hp + 1;
            room->recover(dying.who, recover);
            dying.who->drawCards(hp);
        }
        return false;
    }
};

class SRBiaoxin: public TriggerSkill{
public:
    SRBiaoxin():TriggerSkill("srbiaoxin"){
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        ServerPlayer *to = effect.to;
        if(to == NULL || !to->hasSkill(objectName()))
            return false;
        else;
        const Card *card = effect.card;
        if(card->isKindOf("Dismantlement")){
            int to_throw = room->askForCardChosen(to, to, "hej", objectName());
            room->throwCard(to_throw, to);
            return true;
        }
        if(card->isKindOf("Snatch")){
            int to_give = room->askForCardChosen(to, to, "hej", objectName());
            CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, effect.from->objectName());
            room->moveCardTo(Sanguosha->getCard(to_give), to, effect.from, Player::PlaceHand, reason);
            return true;
        }
        return false;
    }
};

class SRBiaoxinPro: public ProhibitSkill{
public:
    SRBiaoxinPro():ProhibitSkill("#srbiaoxin-pro"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return to->hasSkill("srbiaoxin") && to->isKongcheng() &&
                (card->isKindOf("Duel") || card->isKindOf("Indulgence"));
    }
};

class SRBiaoxinMax: public MaxCardsSkill{
public:
    SRBiaoxinMax():MaxCardsSkill("#srbiaoxin-max"){

    }

    virtual int getExtra(const Player *target) const{
        int x = target->getEquips().length();
        return qMin(2, x);
    }
};

class SRChiji: public TriggerSkill{
public:
    SRChiji():TriggerSkill("srchiji"){
        events << EventPhaseStart << EventPhaseChanging;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case EventPhaseChanging:{
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if(change.to == Player::Judge && !player->isSkipped(Player::Judge) && !player->isSkipped(Player::Draw)
                    && !player->getMark("@speed") && !player->getMark("extraTurn")){
                if(player->askForSkillInvoke(objectName(), data)){
                    player->skip(Player::Judge);
                    player->skip(Player::Draw);
                    player->gainMark("@speed");
                }
            }
            break;
        }
        case EventPhaseStart:{
            if(player->getPhase() == Player::NotActive){
                if(player->getMark("@speed") == 2){
                    player->loseAllMarks("@speed");
                    room->setPlayerMark(player, "extraTurn", 1);
                    player->gainAnExtraTurn();
                }
                if(player->getMark("@speed") == 1){
                    player->gainMark("@speed");
                }
                if(player->getMark("extraTurn")){
                    room->setPlayerMark(player, "extraTurn", 0);
                }
            }
            break;
        }
        }
        return false;
    }
};

class SRSheshen: public TriggerSkill{
public:
    SRSheshen():TriggerSkill("srsheshen"){
        events << Dying;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        ServerPlayer *who = dying.who;
        if(who != player && player->askForSkillInvoke(objectName(), data)){
            room->loseHp(player);
            RecoverStruct reco;
            reco.who = player;
            room->recover(who, reco);
        }
        return false;
    }
};

class SRJianjue: public TriggerSkill{
public:
    SRJianjue():TriggerSkill("srjianjue"){
        events << CardsMoveOneTime << SlashProceed << AfterDrawNCards << EventPhaseEnd;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case CardsMoveOneTime:{
            CardsMoveOneTimeStar moveOneTime = data.value<CardsMoveOneTimeStar>();
            if(moveOneTime->to == NULL) return false;
            if(moveOneTime->to == player && player->getPhase() == Player::Draw && moveOneTime->to_place == Player::PlaceHand){
                foreach(int id, moveOneTime->card_ids){
                    Card *card = Sanguosha->getCard(id);
                    card->setFlags("jianjueDraw");
                }
            }
            break;
        }
        case AfterDrawNCards:{
            if(player->askForSkillInvoke(objectName(), data)){
                QList<const Card *>handCards = player->getHandcards();
                player->addMark("JianjueInvoked");
                foreach(const Card *cd, handCards){
                    if(cd->hasFlag("jianjueDraw"))
                        room->showCard(player, cd->getEffectiveId());
                }
            }
            break;
        }
        case SlashProceed:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(!effect.from->hasSkill(objectName()) || !effect.from->getMark("JianjueInvoked")) return false;
            const Card *slash = room->askForCard(effect.to, "slash", "@jianjue-slash", data, Card::MethodResponse, effect.from);
            room->slashResult(effect, slash);
            return true;
            break;
        }
        case EventPhaseEnd:{
            if(player->getPhase() == Player::Finish){
                if(player->getMark("JianjueInvoked"))
                    room->setPlayerMark(player, "JianjueInvoked", 0);
                foreach(const Card *cd, player->getHandcards()){
                    if(cd->hasFlag("jianjueDraw"))
                        cd->setFlags("-jianjueDraw");
                }
            }
            break;
        }
        default: break;
        }
        return false;
    }
};

class JianjueFilter: public FilterSkill{
public:
    JianjueFilter():FilterSkill("#jianjue-filter"){
        frequency = NotFrequent;
    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped() && to_select->hasFlag("jianjueDraw") && Self->getMark("JianjueInvoked");
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        Slash *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
        slash->setSkillName("srjianjue");
        WrappedCard *card = Sanguosha->getWrappedCard(originalCard->getId());
        card->takeOver(slash);
        return card;
    }
};

class SRLiubi: public TriggerSkill{
public:
    SRLiubi():TriggerSkill("srliubi"){
        events << Damage << CardUsed << Damaged << TurnedOver << SlashMissed << CardResponded << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case Damage:{
            if(!player->hasSkill("lianhuan")){
                DamageStruct dama = data.value<DamageStruct>();
                if(dama.to && player->distanceTo(dama.to) > 1)
                    player->gainMark("lianhuan");
                if(player->getMark("lianhuan") > 1){
                    room->acquireSkill(player, "lianhuan");
                    player->drawCards(2);
                }
            }
            break;
        }
        case CardUsed:{
            if(!player->hasSkill("qiangxi")){
                CardUseStruct use = data.value<CardUseStruct>();
                if(use.card->isKindOf("Slash"))
                    player->gainMark("qiangxi");
                if(player->getMark("qiangxi") > 4){
                    room->acquireSkill(player, "qiangxi");
                    player->drawCards(2);
                }
            }
            break;
        }
        case CardResponded:{
            if(!player->hasSkill("qiangxi")){
                ResponsedStruct res = data.value<ResponsedStruct>();
                if(res.m_card->isKindOf("Slash"))
                    player->gainMark("qiangxi");
                if(player->getMark("qiangxi") > 4){
                    room->acquireSkill(player, "qiangxi");
                    player->drawCards(2);
                }
            }
            break;
        }
        case Damaged:{
            if(!player->hasSkill("ganglie")){
                player->gainMark("ganglie");
            }
            if(player->getMark("ganglie") > 2){
                room->acquireSkill(player, "ganglie");
                player->drawCards(2);
            }
            break;
        }
        case TurnedOver:{
            if(!player->hasSkill("jushou")){
                room->acquireSkill(player, "jushou");
                player->drawCards(2);
            }
            break;
        }
        case CardEffected:{
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(!player->hasSkill("longdan") && effect.to == player){
                const Card *card = effect.card;
                if(card->isKindOf("AOE") || card->isKindOf("Slash") || card->isKindOf("Duel") || card->isKindOf("FireAttack"))
                    player->gainMark("longdan");
                if(player->getMark("longdan") > 5){
                    room->acquireSkill(player, "longdan");
                    player->drawCards(2);
                }
            }
            break;
        }
        case SlashMissed:{
            if(!player->hasSkill("shenwei")){
                player->gainMark("shenwei");
                if(player->getMark("shenwei") > 3){
                    room->acquireSkill(player, "shenwei");
                    player->drawCards(2);
                }
            }
            break;
        }
        default: break;
        }
        return false;
    }
};

class SRShishou: public PhaseChangeSkill{
public:
    SRShishou():PhaseChangeSkill("srshishou"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() != Player::Start)
            return false;
        int x = 0;
        if(player->hasSkill("longdan"))
            x ++;
        if(player->hasSkill("jushou"))
            x ++;
        if(player->hasSkill("shenwei"))
            x ++;
        if(player->hasSkill("ganglie"))
            x ++;
        if(player->hasSkill("qiangxi"))
            x ++;
        if(player->hasSkill("lianhuan"))
            x ++;

        if(x >= 3 && !player->getMark("ShishouLost")){
            room->loseMaxHp(player);
            room->setPlayerMark(player, "ShishouLost", 1);
        }
        if(x >= 4){
            room->detachSkillFromPlayer(player, "srliubi");
            room->detachSkillFromPlayer(player, "srshishou");
        }
        return false;
    }
};

SRMiyinCard::SRMiyinCard(){

}

bool SRMiyinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->getMark("@huimeng");
}

void SRMiyinCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    targets.first()->obtainCard(this);
    targets.first()->gainMark("@huimeng");
}

class SRMiyinView: public OneCardViewAsSkill{
public:
    SRMiyinView():OneCardViewAsSkill("miyin"){

    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->getSuit() == Card::Diamond && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        SRMiyinCard *card = new SRMiyinCard;
        card->addSubcard(originalCard);
        card->setSkillName(objectName());
        return card;
    }
};

class SRMiyin: public TriggerSkill{
public:
    SRMiyin():TriggerSkill("srmiyin"){
        events << EventPhaseStart;
    }

    virtual int getPriority() const{
        return 4;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->getMark("@huimeng");
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(player->getPhase()){
        case Player::Start:{
            QList<const Skill *>skills = player->getVisibleSkillList();
            QStringList skillnames;
            foreach(const Skill *skill, skills){
                skillnames << skill->objectName();
                room->detachSkillFromPlayer(player, skill->objectName());
            }
            room->setTag("Removedskills"+player->objectName(), QVariant(skillnames));
            break;
        }
        case Player::Finish:{
            QStringList skillnames = room->getTag("Removedskills"+player->objectName()).toStringList();
            foreach (QString skillname, skillnames) {
                room->attachSkillToPlayer(player, skillname);
            }
            player->loseAllMarks("@huimeng");
            room->removeTag("Removedskills"+player->objectName());
            break;
        }
        default: return false;
        }
        return false;
    }
};

SRZhaohuanCard::SRZhaohuanCard(){
    target_fixed = true;
}

void SRZhaohuanCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *>targets_choose;
    foreach(ServerPlayer *p, room->getAllPlayers(true)){
        if(!p->isAlive())
            targets_choose << p;
    }

    ServerPlayer *target = room->askForPlayerChosen(source, targets_choose, "srzhaohuan");
    source->loseMark("@zhaohuan");
    room->revivePlayer(target);
    room->changeHero(target, "FireWild", true);
    target->drawCards(2);
    if(source->getRole() == "lord")
        room->setPlayerProperty(target, "role", QVariant("loyalist"));
    else
        room->setPlayerProperty(target, "role", QVariant(source->getRole()));
    room->setPlayerProperty(target, "kingdom", QVariant(source->getKingdom()));
}

class SRZhaohuan: public ZeroCardViewAsSkill{
public:
    SRZhaohuan():ZeroCardViewAsSkill("srzhaohuan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(!player->getMark("@zhaohuan"))
            return false;
        foreach(const Player *p, player->getSiblings()){
            if(!p->isAlive())
                return true;
        }
        return false;
    }

    virtual const Card *viewAs() const{
        return new SRZhaohuanCard;
    }
};

class SRZhuansheng: public TriggerSkill{
public:
    SRZhuansheng():TriggerSkill("srzhuansheng"){
        events << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->isAlive();
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStar dama = data.value<DamageStar>();
        const Card *card = dama->card;
        ServerPlayer *who = room->findPlayerBySkillName(objectName());
        if(dama->from == NULL || dama->from == who || card == NULL || !card->isKindOf("Slash")) return false;
        if(who->faceUp() &&  who->askForSkillInvoke(objectName())){
            int hp = who->getHp();
            room->setTag("hpTarget", QVariant(hp));
            who->turnOver();
            dama->from->turnOver();
            who->addMark("zhuan");
        }
        return false;
    }
};

class SRZhuanshengTri: public TriggerSkill{
public:
    SRZhuanshengTri(): TriggerSkill("#zhuansheng-tri"){
        events << TurnedOver;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if(player->getMark("zhuan") && player->faceUp()){
            int hp = room->getTag("hpTarget").toInt();
            room->removeTag("hpTarget");
            if(player->getHp() < hp){
                LogMessage log;
                log.type = "#ZhuanshengEffect";
                log.from = player;
                room->sendLog(log);
                player->drawCards(hp);
            }
            room->setPlayerMark(player, "zhuan", 0);
        }
        return false;
    }
};

class SRZhuanshengDis: public DistanceSkill{
public:
    SRZhuanshengDis():DistanceSkill("#zhuansheng-dis"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        return (to->hasSkill("srzhuansheng") && !to->faceUp() && from->getHandcardNum() > to->getHandcardNum())?1:0;
    }
};

class SRBilu: public TriggerSkill{
public:
    SRBilu():TriggerSkill("srbilu"){
        events << HpRecover;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        RecoverStruct reco = data.value<RecoverStruct>();
        for(int i = 0; i < reco.recover; i ++){
            if(!player->askForSkillInvoke(objectName()))
                break;
            ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName());
            int x = (player->getHp() < 1)?3:2;// this may cause bug with skills such as Buqu and NosBuqu
            target->drawCards(x);
        }
        return false;
    }
};

class SRZongyuan: public DistanceSkill{
public:
    SRZongyuan():DistanceSkill("srzongyuan"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(from->hasSkill(objectName()) && from->inMyAttackRange(to))
            return -99;
        else
            return 0;
    }
};

class SRKeyin: public TriggerSkill{
public:
    SRKeyin():TriggerSkill("srkeyin"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        const Slash *slash = effect.slash;
        QList<ServerPlayer *>targets;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            if(player->inMyAttackRange(p) && !player->isProhibited(p, slash) && p != effect.to)
                targets << p;//the skill card used in lua skill can be used to someone who is prohibited.
        }
        if(!targets.isEmpty() && player->askForSkillInvoke(objectName(), data)){
            ServerPlayer *to = room->askForPlayerChosen(player, targets, objectName());
            CardUseStruct use;
            use.from = player;
            use.to << to;
            use.card = slash;
            room->useCard(use);
        }
        return false;
    }
};

class SRXishi: public MasochismSkill{
public:
    SRXishi():MasochismSkill("srxishi"){
        frequency = Compulsory;
    }

    virtual void onDamaged(ServerPlayer *target, const DamageStruct &damage) const{
        if(target->isKongcheng()) return;
        Room *room = target->getRoom();
        int total = 0, count = 0;
        QList<int>card_ids;
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            if(!p->isKongcheng()){
                const Card *card = room->askForExchange(p, objectName(), 1, false, "@srxishi", false);
                CardMoveReason reason(CardMoveReason::S_REASON_RESPONSE, p->objectName());
                room->moveCardTo(card, NULL, Player::PlaceTable, reason, true);
                LogMessage log;
                log.from = p;
                log.arg = card->getNumberString();
                log.type = "#respond";
                room->sendLog(log);
                total += card->getNumber();
                count ++;
                card_ids << card->getEffectiveId();
                if(total > 21){
                    LogMessage log;
                    log.from = p;
                    log.type = "#damage";
                    log.arg = total;
                    room->sendLog(log);
                    room->loseHp(p);
                    CardsMoveStruct move;
                    move.card_ids = card_ids;
                    move.to_place = Player::DiscardPile;
                    room->moveCardsAtomic(move, true);
                    if(damage.from && damage.from != p && !damage.from->isNude()){
                        LogMessage log;
                        log.from = p;
                        log.type = "#obtain";
                        room->sendLog(log);
                        int id = room->askForCardChosen(p, damage.from, "he", objectName());
                        room->obtainCard(p, id, room->getCardPlace(id) == Player::PlaceEquip);
                    }
                    return;
                }
                if(total == 21){
                    p->drawCards(count);
                    target->drawCards(count);
                    CardsMoveStruct move;
                    move.card_ids = card_ids;
                    move.to_place = Player::DiscardPile;
                    room->moveCardsAtomic(move, true);
                    return;
                }
            }
        }
        if(count > 2) count = 2;
        QList<int>to_get;
        for(int i = 0; i < count; i ++){
            room->fillAG(card_ids, target);
            int id = room->askForAG(target, card_ids, false, objectName());
            target->invoke("clearAG");
            to_get << id;
            card_ids.removeOne(id);
        }
        CardsMoveStruct get, thr;
        get.card_ids = to_get;
        thr.card_ids = card_ids;
        CardMoveReason reason(CardMoveReason::S_REASON_GOTBACK, target->objectName());
        get.reason = reason;
        get.to = target;
        get.to_place = Player::PlaceHand;
        thr.to_place = Player::DiscardPile;
        QList<CardsMoveStruct> moves;
        moves.push_back(get);
        moves.push_back(thr);
        room->moveCardsAtomic(moves, true);
        return;
    }
};

class SRZizhu: public TriggerSkill{
public:
    SRZizhu():TriggerSkill("srzizhu"){
        events << EventPhaseStart;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(player->getPhase()){
        case Player::Start:{
            int hcn = player->getHandcardNum();
            bool jishu = (hcn%2)?true:false;
            QList<ServerPlayer *>targets;
            foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                bool ji = ((p->getHandcardNum())%2)?true:false;
                if(ji != jishu && p->getHandcardNum() > hcn)
                    targets << p;
            }
            if(!targets.isEmpty() && player->askForSkillInvoke(objectName())){
                room->broadcastSkillInvoke(objectName(), 1);
                ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
                room->setPlayerMark(player, "threeInvoke", hcn + 1);
                int drawNum = target->getHandcardNum() - hcn;
                player->drawCards(drawNum);
                if(drawNum > 3)
                    player->skip(Player::Draw);
            }
            break;
        }
        case Player::Finish:{
            if(player->getMark("threeInvoke") > 0){
                int hc = player->getMark("threeInvoke") - 1;
                int nhc = player->getHandcardNum();
                int y = qAbs(hc - nhc);
                room->setPlayerMark(player, "threeInvoke", 0);
                if(y%2 == 1){
                    LogMessage log;
                    log.type = "#TriggerSkill";
                    log.from = player;
                    log.arg = objectName();
                    room->sendLog(log);
                    if(nhc == 0){
                        room->broadcastSkillInvoke(objectName(), 2);
                        room->loseHp(player);
                    }else{
                        room->broadcastSkillInvoke(objectName(), 3);
                        room->askForDiscard(player, objectName(), 1, 1);
                    }
                }
            }
            break;
        }
        default: return false;
        }
        return false;
    }
};

class SRChuanyun: public TriggerSkill{
public:
    SRChuanyun():TriggerSkill("srchuanyun"){
        events << DamageCaused;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStar dama = data.value<DamageStar>();
        ServerPlayer *to = dama->to;
        if(dama->chain || dama->transfer || dama->card == NULL || !dama->card->isKindOf("Slash")) return false;
        QList<ServerPlayer *>targets;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            if(!p->isNude() && p != to)
                targets << p;
        }
        QVariant tag;
        tag.setValue(to);
        room->setTag("CYvictim", tag);
        if(!targets.isEmpty() && player->askForSkillInvoke(objectName())){
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
            if(player->inMyAttackRange(target)  && to->inMyAttackRange(target)){
                room->broadcastSkillInvoke(objectName(), 1);
                room->askForDiscard(target, objectName(), qMin(2, target->getCardCount(true)),
                                    qMin(2, target->getCardCount(true)), false, true);
            }else{
                room->broadcastSkillInvoke(objectName(), 2);
                room->askForDiscard(target, objectName(), qMin(2, target->getCardCount(true)),
                                    qMin(2, target->getCardCount(true)), false, true);
                return true;
            }
        }
        room->removeTag("CYvictim");
        return false;
    }
};

class SRJianyan: public TriggerSkill{
public:
    SRJianyan():TriggerSkill("srjianyan"){
        events << EventPhaseStart;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::Discard) return false;
        if(player->getHandcardNum() > player->getMaxCards() && player->askForSkillInvoke(objectName())){
            room->showAllCards(player);
            QList<const Card *>allCards = player->getHandcards();
            QList<int>to_dis;
            foreach(const Card *cd, allCards){
                if(cd->isRed())
                    to_dis << cd->getEffectiveId();
            }
            if(to_dis.length() > player->getMaxCards()){
                int dis_num = to_dis.length() - player->getMaxCards();
                for(int i = 0; i < dis_num; i ++){
                    room->fillAG(to_dis, player);
                    int dis = room->askForAG(player, to_dis, false, objectName());
                    to_dis.removeOne(dis);
                    room->throwCard(dis, player);
                    player->invoke("clearAG");
                }
                return true;
            }else
                return true;
        }
        return false;
    }
};

SRGuifuCard::SRGuifuCard(){

}

bool SRGuifuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void SRGuifuCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    source->loseMark("@zuzhou");
    ServerPlayer *dest = targets.first();
    dest->gainMark("@curse");
    room->loseHp(dest);
    room->broadcastInvoke("animate", "lightbox:$AyGuiFu");
}

class SRGuifuView: public ViewAsSkill{
public:
    SRGuifuView():ViewAsSkill("srguifu"){

    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
        if(selected.isEmpty())
            return true;
        if(selected.length() == 1)
            return to_select->getSuit() == selected.first()->getSuit();
        return false;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
        if(cards.length() < 2)
            return NULL;
        SRGuifuCard *card = new SRGuifuCard;
        card->addSubcards(cards);
        card->setSkillName(objectName());
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@zuzhou") && player->getCardCount(true) >= 2;
    }
};

class SRGuifu: public TriggerSkill{
public:
    SRGuifu():TriggerSkill("srguifu"){
        events << TurnStart;
        view_as_skill = new SRGuifuView;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->isAlive() && target->getMark("@curse");
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        JudgeStruct judge;
        judge.who = player;
        judge.pattern = QRegExp("(.*):(spade):([2-9])");
        judge.reason = objectName();
        judge.play_animation = true;
        judge.good = false;
        if(judge.isBad()){
            int hp = player->getHp();
            if(hp > 0){
                room->loseHp(player, hp);
                player->drawCards(hp);
            }
            player->loseAllMarks("@curse");
        }
        return false;
    }
};

class SRHaoyong: public TriggerSkill{
public:
    SRHaoyong():TriggerSkill("srhaoyong"){
        events << SlashEffect;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        ServerPlayer *who = room->findPlayerBySkillName(objectName());
        if(player->getPhase() == Player::Play && !player->hasUsed("Analeptic") && who && effect.to != who &&
                !who->isNude()){
            if(!effect.slash->hasFlag("drank")){
                QVariant ai_data;
                ai_data.setValue(effect.to);
                if(room->askForSkillInvoke(who, objectName(), ai_data)){
                    if(room->askForCard(who, "slash,Weapon", "@jiajiu")){
                        effect.slash->setFlags("drank");
                    }
                }
            }
        }
        return false;
    }
};

class SRZhige: public TriggerSkill{
public:
    SRZhige():TriggerSkill("srzhige"){
        frequency = Wake;
        events << Death;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        DeathStruct death = data.value<DeathStruct>();
        ServerPlayer *source = room->findPlayerBySkillName(objectName());
        if(source = death.who) return false;
        QString kingdom = death.who->getKingdom();
        bool can = true;
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            if(p->getKingdom() == kingdom){
                can = false;
                break;
            }
        }
        if(can){
            if(source->isAlive() && !source->getMark("zhige")){
                source->gainMark("@waked");
                room->setPlayerMark(source, "zhige", 1);
                if(!source->getEquips().isEmpty()){
                    int x = source->getEquips().length();
                    player->drawCards(x);
                    CardsMoveStruct move;
                    move.to_place = Player::DiscardPile;
                    foreach(const Card *cd, source->getEquips())
                        move.card_ids << cd->getEffectiveId();
                    CardMoveReason reason(CardMoveReason::S_REASON_THROW, source->objectName());
                    room->moveCards(move, true);
                }
                room->acquireSkill(source, "yuanxing");//this need to be changed according to others code
            }
        }
        return false;
    }
};

SRBiheCard::SRBiheCard(){

}

bool SRBiheCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && to_select->getWeapon();
}

void SRBiheCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    ServerPlayer *dest = targets.first();
    dest->addToPile("srbihe", subcards.first(), false);
    CardMoveReason reason(CardMoveReason::S_REASON_PUT, source->objectName(), "srbihe", "");
    room->moveCardTo(dest->getWeapon(), dest, source, Player::PlaceEquip, reason, true);
}

class SRBiheView: public OneCardViewAsSkill{
public:
    SRBiheView():OneCardViewAsSkill("srbihe"){

    }

    virtual bool viewFilter(const Card *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        foreach(const Player *p, player->getSiblings()){
            if(!p->getPile("srbihe").isEmpty())
                return false;
        }
        return player->getWeapon() == NULL;
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        SRBiheCard *card = new SRBiheCard;
        card->addSubcard(originalCard);
        card->setSkillName(objectName());
        return card;
    }
};

class SRBihe: public TriggerSkill{
public:
    SRBihe():TriggerSkill("srbihe"){
        events << EventPhaseStart << CardsMoveOneTime << EventLoseSkill;
        view_as_skill = new SRBiheView;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case EventPhaseStart:{
            if(player->getPhase() == Player::Start && player->getWeapon() == NULL){
                CardMoveReason reason(CardMoveReason::S_REASON_PUT, player->objectName(), objectName(), "");
                if(player->hasSkill(objectName())){
                    foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                        if(!p->getPile("srbihe").isEmpty() && p->getWeapon()){
                            room->moveCardTo(p->getWeapon(), p, player, Player::PlaceEquip, reason, true);
                        }
                    }
                }else{
                    if(!player->getPile("srbihe").isEmpty()){
                        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                            if(p->hasSkill(objectName()) && p->getWeapon()){
                                room->moveCardTo(p->getWeapon(), p, player, Player::PlaceEquip, reason, true);
                            }
                        }
                    }
                }
            }
            break;
        }
        case CardsMoveOneTime:{
            CardsMoveOneTimeStar move = data.value<CardsMoveOneTimeStar>();
            if(move->from_places.contains(Player::PlaceEquip) && player->getWeapon() == NULL){
                if(player->hasSkill(objectName())){
                    foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                        if(!p->getPile("srbihe").isEmpty() && !p->getWeapon())
                            p->removePileByName("srbihe");
                    }
                }else{
                    if(player->getPile("srbihe").length() > 0){
                        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                            if(p->hasSkill(objectName()) && !p->getWeapon())
                                player->removePileByName("srbihe");
                        }
                    }
                }
            }
            break;
        }
        case EventLoseSkill:{
            if(data.toStringList().contains(objectName())){
                foreach(ServerPlayer *p, room->getAlivePlayers()){
                    if(!p->getPile("srbihe").isEmpty())
                        p->removePileByName("srbihe");
                }
            }
            break;
        }
        default: break;
        }
        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName()) || target->getPile("srbihe").length() > 0;
    }
};

class SRGongying: public PhaseChangeSkill{
public:
    SRGongying():PhaseChangeSkill("srgongying"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() != Player::Finish) return false;
        Room *room = target->getRoom();
        QList<ServerPlayer *>targets;
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            if(p->getHandcardNum() < p->getHp())
                targets << p;
        }
        if(targets.isEmpty() || !target->askForSkillInvoke(objectName())) return false;
        ServerPlayer *dest = room->askForPlayerChosen(target, targets, objectName());
        room->broadcastSkillInvoke(objectName());
        dest->drawCards(1);
        return false;
    }
};

class SRGuili: public ProhibitSkill{
public:
    SRGuili():ProhibitSkill("srguili"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return to->hasSkill(objectName()) && !from->getMark("srguili") &&
                (card->isKindOf("Duel") || card->isKindOf("Collateral") || card->isKindOf("ThunderSlash") || card->isKindOf("FireSlash"));
    }
};

class SRGuiliAdd: public TriggerSkill{
public:
    SRGuiliAdd():TriggerSkill("#srguili-add"){
        frequency = Compulsory;
        events << TargetConfirmed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->hasSkill(objectName()) && target->isAlive();
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        const Card *card = use.card;
        if(use.from == player && (card->isKindOf("Duel") || card->isKindOf("Snatch"))){
            foreach(ServerPlayer *p, use.to){
                if(!p->getMark("srguili"))
                    p->gainMark("srguili");
            }
        }
        return false;
    }
};

class SRYuxue: public TriggerSkill{
public:
    SRYuxue():TriggerSkill("sryuxue"){
        events << Damaged << Dying;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case Damaged:{
            DamageStar dama = data.value<DamageStar>();
            if(!dama->from || dama->from->isDead()) return false;
            Slash *slash = new Slash(Card::NoSuit, 0);
            if(player->isProhibited(dama->from, slash)) return false;
            delete slash;
            if(player->askForSkillInvoke(objectName())){
                room->broadcastSkillInvoke(objectName(), qrand()%2 + 1);
                for(int i = 0; i < player->getLostHp(); i ++){
                    if(dama->from->isDead()) break;
                    JudgeStruct judge;
                    judge.reason = objectName();
                    judge.who = player;
                    judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                    judge.good = true;
                    judge.play_animation = true;
                    room->judge(judge);
                    if(judge.isGood() && dama->from->isAlive()){
                        Slash *slash = new Slash(Card::NoSuit, 0);
                        slash->setSkillName(objectName());
                        CardUseStruct use;
                        use.card = slash;
                        use.from = player;
                        use.to << dama->from;
                        room->useCard(use, false);
                    }
                }
            }
            break;
        }
        case Dying:{
            DyingStruct dying = data.value<DyingStruct>();
            if(dying.who->hasSkill(objectName())){
                room->broadcastSkillInvoke(objectName(), 3);
                dying.who->drawCards(3);
            }
            break;
        }
        }
        return false;
    }
};

class SRShizhang: public TriggerSkill{
public:
    SRShizhang():TriggerSkill("srshizhang"){
        frequency = Compulsory;
        events << TargetConfirming;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->isKindOf("Slash") || !use.to.contains(player)) return false;
        int x = qrand()%4 + 1;
        if(x == 1){
            QList<ServerPlayer *>targets = room->getAlivePlayers();
            int n = targets.length();
            if(n > 0){
                int a = qrand()%(n - 1);
                ServerPlayer *target = targets.at(a);
                if(target != player){
                    use.to.insert(use.to.indexOf(player), target);
                    use.to.removeOne(player);
                    room->broadcastSkillInvoke(objectName(), target->isMale()?1:2);
                    LogMessage log;
                    log.type = "#ShiZhang";
                    log.from = player;
                    log.arg = target->getGeneralName();
                    room->sendLog(log);
                    data = QVariant::fromValue(use);
                    return true;
                }
            }
        }
        return false;
    }
};

class SRTudu: public TriggerSkill{
public:
    SRTudu():TriggerSkill("srtudu"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStar dama = data.value<DamageStar>();
        if(dama->to == NULL || dama->to->isDead()) return false;
        if(!dama->to->getMark("@poison")){
            int x = qrand()%((player->getDefensiveHorse() || player->getOffensiveHorse())?10:15);
            if(x <= 6 && player->askForSkillInvoke(objectName())){
                dama->to->gainMark("@poison");
            }
        }
        return false;
    }
};

class SRTuduEffect: public DrawCardsSkill{
public:
    SRTuduEffect():DrawCardsSkill("#srtudu-effect"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->getMark("@poison");
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        if(player->isKongcheng()){
            player->loseAllMarks("@poison");
            return n;
        }else{
            Room *room = player->getRoom();
            if(room->askForCard(player, "Slash,Analeptic", "@AskForJiedu")){
                player->loseAllMarks("@poison");
                return n;
            }else
                return 1;
        }
        return 1;
    }
};

class SRFenglue: public TriggerSkill{
public:
    SRFenglue():TriggerSkill("srfenglue"){
        events << CardsMoveOneTime;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        QList<int>cdlist;
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        ServerPlayer *source = room->findPlayerBySkillName(objectName());
        ServerPlayer *current = room->getCurrent();
        if(current != player){
            if(move.to && (move.to_place == Player::PlaceHand || move.to_place == Player::PlaceEquip)
                    && move.to == player && (!move.from || move.from != player)){
                cdlist = move.card_ids;
            }
        }
        if(cdlist.isEmpty() || !source ||source->isNude()) return false;
        QVariant ai_data;
        ai_data.setValue(player);
        foreach(int id, cdlist){
            if(!room->askForSkillInvoke(source, objectName(), ai_data) ||
                    !room->askForCard(source, ".|spade,club|.|.", "@fenglue")) return false;
            room->obtainCard(source, id, true);
        }
        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }
};

class SRDuntuo: public TriggerSkill{
public:
    SRDuntuo():TriggerSkill("srduntuo"){
        events << Damaged << EventPhaseStart;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case Damaged:{
            DamageStar damage = data.value<DamageStar>();
            if(damage->damage == 1 && !player->getMark("@escape") && player->askForSkillInvoke(objectName()))
                player->gainMark("@escape");
            break;
        }
        case EventPhaseStart:{
            if(player->getPhase() == Player::Finish && player->getMark("@escape"))
                player->loseAllMarks("@escape");
            break;
        }
        }
        return false;
    }
};

class SRDuntuoDis: public DistanceSkill{
public:
    SRDuntuoDis():DistanceSkill("#srduntuo-dis"){
        frequency = NotFrequent;
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(from->getMark("@escape") || to->getMark("@escape"))
            return 999;
        else{
            QList<const Player*>sib = from->getSiblings();
            bool n = false;
            foreach(const Player *p, sib){
                if(p->isAlive() && p->getMark("@escape")){
                    n = true;
                    break;
                }
            }
            if(n){
                QList<const Player*>others;
                int fromseat = from->getSeat();
                foreach(const Player *p, sib){
                    if(p->getSeat() > fromseat && p->isAlive())
                        others.append(p);
                }
                foreach(const Player *p, sib){
                    if(p->getSeat() < fromseat && p->isAlive())
                        others.append(p);
                }
                int x = others.length();
                int semi = (x - 1)/2;
                int toseat = -1;
                for(int i = 0; i < others.length(); i++){
                    if(others.at(i) == to){
                        toseat = i;
                        break;
                    }
                }
                if(toseat < semi){
                    int y = 0;
                    for(int i = 0; i < toseat; i ++){
                        const Player *p = others.at(i);
                        if(p->isAlive() && p->getMark("@escape"))
                            y ++;
                    }
                    return -y;
                }else{
                    if(toseat > semi){
                        int y = 0;
                        for(int i = others.length() - 1; i > toseat; i --){
                            const Player *p = others.at(i);
                            if(p->isAlive() && p->getMark("@escape"))
                                y ++;
                        }
                        return -y;
                    }else{
                        int y1 = 0, y2 = 0;
                        for(int i = 0; i < others.length(); i ++){
                            const Player *p = others.at(i);
                            if(p->isAlive() && p->getMark("@escape")){
                                if(i > semi)
                                    y1 ++;
                                if(i < semi)
                                    y2 ++;
                            }
                        }
                        return -qMax(y1, y2);
                    }
                }
            }
        }
        return 0;
    }
};

SRJiahuoCard::SRJiahuoCard(){
    target_fixed = true;
}

class SRJiahuoView: public OneCardViewAsSkill{
public:
    SRJiahuoView():OneCardViewAsSkill("srjianhuo"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@srjiahuo" && !player->hasUsed("SRJiahuoCard");
    }

    virtual bool viewFilter(const Card *to_select) const{
        return to_select->isRed() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const Card *originalCard) const{
        SRJiahuoCard *card = new SRJiahuoCard;
        card->addSubcard(originalCard);
        card->setSkillName(objectName());
        return card;
    }
};

class SRJiahuo: public TriggerSkill{
public:
    SRJiahuo():TriggerSkill("srjiahuo"){
        view_as_skill = new SRJiahuoView;
        events << TargetConfirmed << CardEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        Room *room = target->getRoom();
        ServerPlayer *source = room->findPlayerBySkillName(objectName());
        return source && source->isAlive() && !source->isKongcheng();
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *source = room->findPlayerBySkillName(objectName());
        switch(event){
        case TargetConfirmed:{
            CardUseStruct use = data.value<CardUseStruct>();
            QList<ServerPlayer *>targets = use.to;
            const Card *card = use.card;
            ServerPlayer *target;
            if(targets.length() == 1){
                target = targets.first();
                if(card->isNDTrick() && !target->hasFlag(objectName()) && room->askForUseCard(source, "@srjiahuo", "@@srjiahuo")){
                    room->setPlayerFlag(target, objectName());
                }
            }
            break;
        }
        case CardEffected:{
            CardEffectStruct effect = data.value<CardEffectStruct>();
            const Card *card = effect.card;
            ServerPlayer *from = effect.from, *to = effect.to;
            if(to->hasFlag(objectName()) && card->isNDTrick()){
                room->setPlayerFlag(to, "-srjiahuo");
                Slash *slash = new Slash(card->getSuit(), card->getNumber());
                slash->addSubcard(card);
                slash->setSkillName(objectName());
                CardUseStruct use;
                use.card = slash;
                use.from = from;
                use.to << to;
                room->useCard(use);
                return true;
            }
            break;
        }
        }
        return false;
    }
};

class SRXihua: public DrawCardsSkill{
public:
    SRXihua():DrawCardsSkill("srxihua"){
        frequency = Wake;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        Room *room = player->getRoom();
        int count = 0;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            if(p->isAlive() && !p->isMale())
                count ++;
        }
        if(count == 1){
            room->acquireSkill(player, "yuanhu");
            room->acquireSkill(player, "nosjiefan");
            room->loseMaxHp(player);
            player->drawCards(2);
            player->gainMark("@waked");
        }
        return n;
    }
};



SwordRainPackage::SwordRainPackage()
    :Package("swordrain")
{
    General *sryueru, *srsuyu, *srzixuan, *spmengli, *srxuanyu, *srlengtu, *srqishuang, *splinger, *splingsha;

    sryueru = new General(this, "sryueru", "wei", 4, false);
    sryueru->addSkill(new SRJie);

    srsuyu = new General(this, "srsuyu", "wei", 3, false);
    srsuyu->addSkill(new SRLengyue);
    srsuyu->addSkill(new SRYushang);
    srsuyu->addSkill(new SRYushangTri);
    srsuyu->addSkill(new SRBingyun);
    related_skills.insertMulti("sryushang", "#sryushang");

    srzixuan = new General(this, "srzixuan", "wei", 4, false);
    srzixuan->addSkill(new SRWugu);
    srzixuan->addSkill(new SRWuguGet);
    srzixuan->addSkill(new SRJiejin);
    srzixuan->addRelateSkill("srjuling");

    spmengli = new General(this, "spmengli", "wu", 3, false);
    spmengli->addSkill(new SRJingxiang);
    spmengli->addSkill(new SRHuanming);

    srxuanyu = new General(this, "srxuanyu", "qun", 3, false);
    srxuanyu->addSkill(new SRYuyan);
    srxuanyu->addSkill(new SRMeixi);

    srlengtu = new General(this, "srlengtu", "qun", 3, true);
    srlengtu->addSkill(new SRHongfu);
    srlengtu->addSkill(new SRHongfuClear);
    srlengtu->addSkill(new SRQushi);
    srlengtu->addSkill(new SRQushiRecord);
    related_skills.insertMulti("srhongfu", "#srhongfu-clear");
    related_skills.insertMulti("srqushi", "#srqushi-record");

    srqishuang = new General(this, "srqishuang", "wei", 3, false);
    srqishuang->addSkill(new SRLiangshuang);
    srqishuang->addSkill(new SRFazhen);

    splinger = new General(this, "splinger", "wei", 4, false);
    splinger->addSkill(new SRDoubleWeapon);
    splinger->addSkill(new SRDoubleSlash);
    splinger->addSkill(new SRWuling);
    splinger->addSkill(new SRLeizhouEffect);
    related_skills.insertMulti("srdoubleweapon", "#srdouble");
    related_skills.insertMulti("srwuling", "#leizhoueffect");

    splingsha = new General(this, "splingsha", "wei", 3, false);
    splingsha->addSkill(new SRYidao);
    splingsha->addSkill(new SRQingdeng);

    General *srlixiaoyao, *FireWild, *srcaiyi, *srbiaoshi, *spyueru, *zhenyumingwang, *sranu, *srxiyao;

    srlixiaoyao = new General(this, "srlixiaoyao", "shu");
    srlixiaoyao->addSkill(new SRTanyun);
    srlixiaoyao->addSkill(new SRTanyunTar);
    srlixiaoyao->addSkill(new SRZhangjian);
    related_skills.insertMulti("srtanyun", "#srtanyun-tar");

    FireWild = new General(this, "FireWild", "god", 2, true, true);
    FireWild->setGender(General::SexLess);
    FireWild->addSkill(new SRFenwu);
    FireWild->addSkill(new SRHuolin);

    srcaiyi = new General(this, "srcaiyi", "wu", 3, false);
    srcaiyi->addSkill(new SRHuimeng);
    srcaiyi->addSkill(new SRHuimengBack);
    srcaiyi->addSkill(new SRDielian);
    related_skills.insertMulti("srhuimeng", "#srhuimeng");

    srbiaoshi = new General(this, "srbiaoshi", "qun", 3);
    srbiaoshi->addSkill(new SRBiaoxin);
    srbiaoshi->addSkill(new SRBiaoxinMax);
    srbiaoshi->addSkill(new SRBiaoxinPro);
    srbiaoshi->addSkill(new SRChiji);
    related_skills.insertMulti("srbiaoshi", "#srbiaoxin-max");
    related_skills.insertMulti("srbiaoxin", "#srbiaoxin-pro");

    spyueru = new General(this, "spyueru", "qun", 4, false);
    spyueru->addSkill(new SRSheshen);
    spyueru->addSkill(new SRJianjue);
    spyueru->addSkill(new JianjueFilter);
    related_skills.insertMulti("srjianjue", "#jianjue-filter");

    zhenyumingwang = new General(this, "zhenyumingwang", "god");
    zhenyumingwang->addSkill(new SRLiubi);
    zhenyumingwang->addSkill(new SRShishou);

    sranu = new General(this, "anu", "wu", 3, false, true);
    sranu->addSkill(new SRMiyin);
    sranu->addSkill(new SRZhaohuan);
    sranu->addSkill(new MarkAssignSkill("@zhaohuan", 1));
    related_skills.insertMulti("srzhaohuan", "#@zhaohuan-1");

    srxiyao = new General(this, "srxiyao", "wei", 3, false);
    srxiyao->addSkill(new SRZhuansheng);
    srxiyao->addSkill(new SRZhuanshengDis);
    srxiyao->addSkill(new SRZhuanshengTri);
    srxiyao->addSkill(new SRBilu);
    related_skills.insertMulti("srzhuansheng", "#zhuansheng-tri");
    related_skills.insertMulti("srzhuansheng", "#zhuansheng-dis");

    General *srchonglou, *srjingtian, *srwangxiaohu, *srleiyuange, *srwenhui, *srtiansha, *srchigui;

    srchonglou = new General(this, "srchonglou", "god");
    srchonglou->addSkill(new SRZongyuan);
    srchonglou->addSkill(new SRKeyin);

    srjingtian = new General(this, "srjingtian", "wu");
    srjingtian->addSkill(new SRXishi);
    srjingtian->addSkill(new SRZizhu);

    srwangxiaohu = new General(this, "srwangxiaohu", "wu", 4, true, true);
    srwangxiaohu->addSkill(new SRChuanyun);

    srleiyuange = new General(this, "srleiyuange", "wu");
    srleiyuange->addSkill(new SRJianyan);
    srleiyuange->addSkill(new SRGuifu);
    srleiyuange->addSkill(new MarkAssignSkill("@zuzhou", 1));
    related_skills.insertMulti("srguifu", "#@zuzhou-1");

    srwenhui = new General(this, "srwenhui", "shu", 4, false);
    srwenhui->addSkill(new SRHaoyong);
    srwenhui->addSkill(new SRZhige);
    //srwenhui->addRelateSkill("yuanxing");//this also need to change according to other's codes

    srtiansha = new General(this, "srtiansha", "shu", 3);
    srtiansha->addSkill(new SRBihe);
    srtiansha->addSkill(new SRGongying);
    srtiansha->addSkill(new SRGuili);
    srtiansha->addSkill(new SRGuiliAdd);
    related_skills.insertMulti("srguili", "#srguili-add");

    srchigui = new General(this, "srchigui", "shu", 3);
    srchigui->addSkill(new SRYuxue);
    srchigui->addSkill(new SRShizhang);

    General *srguimu, *srfeizei, *spjingtian, *sptianhe;

    srguimu = new General(this, "srguimu", "god", 4, false);
    srguimu->addSkill(new SRTudu);
    srguimu->addSkill(new SRTuduEffect);
    related_skills.insertMulti("srtudu", "#srtudu-effect");

    srfeizei = new General(this, "srfeizei", "qun", 3, false);
    srfeizei->addSkill(new SRFenglue);
    srfeizei->addSkill(new SRDuntuo);
    srfeizei->addSkill(new SRDuntuoDis);
    srfeizei->addSkill(new SRJiahuo);
    related_skills.insertMulti("srduntuo", "#srduntuo-dis");

    spjingtian = new General(this, "spjingtian", "wu");

    sptianhe = new General(this, "sptianhe", "wei", 4, true, true);
    sptianhe->addSkill(new SRXihua);

    skills << new SRJuling;

    addMetaObject<SRLengyueCard>();
    addMetaObject<SRYuyanCard>();
    addMetaObject<SRYidaoCard>();
    addMetaObject<SRFenwuCard>();
    addMetaObject<SRMiyinCard>();
    addMetaObject<SRGuifuCard>();
    addMetaObject<SRBiheCard>();
    addMetaObject<SRJiahuoCard>();
}

ADD_PACKAGE(SwordRain)
