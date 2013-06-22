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

#endif // SWORDRAIN_H
