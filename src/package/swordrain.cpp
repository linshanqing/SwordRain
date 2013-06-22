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
        if(!damage.from || !damage.to || damage.from->isKongcheng() || target->isKongcheng())
            return;
        if(target->askForSkillInvoke(objectName())){
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

class SRQianjin: public PhaseChangeSkill{
public:
    SRQianjin():PhaseChangeSkill("srqianjin"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(!target->isKongcheng() || target->getPhase() != Player::Start)
            return false;
        int x = target->getHp() - target->getHandcardNum();
        if(x <= 0) return false;

        LogMessage log;
        log.type = "#TriggerSkill";
        log.arg = objectName();
        log.from = target;
        Room* room = target->getRoom();
        room->broadcastSkillInvoke(objectName());
        room->sendLog(log);

        target->drawCards(x);
        return false;
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
                room->setPlayerMark(source, "wugudr", 1);
                data = QVariant::fromValue(reco);
            }
            break;
        }
        case HpRecover:{
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
            if(p->getHandcardNum() < p->getMaxHp() && p->askForSkillInvoke(objectName())){
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
        room->acquireSkill(target, "srjiling");
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
            if(!mings.isEmpty() && player->askForSkillInvoke(objectName()) && player->getPhase() == Player::Finish){
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

SwordRainPackage::SwordRainPackage()
    :Package("swordrain")
{
    General *sryueru, *srsuyu, *srzixuan, *spmengli;

    sryueru = new General(this, "sryueru", "wei", 4, false);
    sryueru->addSkill(new SRJie);
    sryueru->addSkill(new SRQianjin);

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

    skills << new SRJuling;

    addMetaObject<SRLengyueCard>();
}

ADD_PACKAGE(SwordRain)