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
        TanyunSnatch *snatch = new TanyunSnatch(originalCard->getSuit(), originalCard->getNumber());
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
            Card *card = Sanguosha->getCard(id);
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
            room->moveCards(move);
        }else{
            foreach(int id, ids){
                duel->addSubcard(id);
            }
            CardUseStruct use;
            use.card = duel;
            use.from = player;
            use.to = room->askForPlayerChosen(player, dests, objectName());
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
    use1.to = second;
    use2.from = second;
    use2.card = slash2;
    use2.to = first;
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
                room->moveCardTo(Sanguosha->getCard(i), players[i], places[i], , true);
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
            CardMoveReason reason(CardMoveReason::S_REASON_EXTRACTION, effect.from);
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
            room->recover(who, roco);
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

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
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

    General *srlixiaoyao, *FireWild, *srcaiyi, *srbiaoshi, *spyueru, *zhenyumingwang;

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

    skills << new SRJuling;

    addMetaObject<SRLengyueCard>();
    addMetaObject<SRYuyanCard>();
    addMetaObject<SRYidaoCard>();
    addMetaObject<SRFenwuCard>();
}

ADD_PACKAGE(SwordRain)
