#ifndef SWORDRAIN_H
#define SWORDRAIN_H

#include "package.h"
#include "card.h"

class SwordRainPackage: public Package{
    Q_OBJECT
public:
    SwordRainPackage();
};

class SRLengyueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SRLengyueCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class SRYuyanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SRYuyanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class SRYidaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SRYidaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class SRFenwuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SRFenwuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class SRMiyinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SRMiyinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class SRZhaohuanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SRZhaohuanCard();

    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

class SRGuifuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SRGuifuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
};

#endif // SWORDRAIN_H
