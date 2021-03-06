#ifndef STANDARDEQUIPS_H
#define STANDARDEQUIPS_H

#include "standard.h"

class Crossbow:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Crossbow(Card::Suit suit, int number = 1);
};

class DoubleSword:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE DoubleSword(Card::Suit suit = Spade, int number = 2);
};

class QinggangSword:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE QinggangSword(Card::Suit suit = Spade, int number = 6);
};

//class Blade:public Weapon{
//    Q_OBJECT

//public:
//    Q_INVOKABLE Blade(Card::Suit suit = Spade, int number = 5);
//};

//class Spear:public Weapon{
//    Q_OBJECT

//public:
//    Q_INVOKABLE Spear(Card::Suit suit = Spade, int number = 12);
//};

class Axe:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Axe(Card::Suit suit = Diamond, int number = 5);
};

class Halberd:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Halberd(Card::Suit suit = Diamond, int number = 12);
};

class KylinBow:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE KylinBow(Card::Suit suit = Heart, int number = 5);
};

class BlackHurt:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE BlackHurt(Card::Suit suit = Spade, int number = 2);
};

class MagicSword:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE MagicSword(Card::Suit suit = Diamond, int number = 9);
};

class KongHou:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE KongHou(Card::Suit suit = Club, int number = 6);
};

class Hook:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Hook(Card::Suit suit = Heart, int number = 11);
};

class EightDiagram:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE EightDiagram(Card::Suit suit, int number = 2);
};


class RenwangShield: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE RenwangShield(Card::Suit suit, int number);
};

class StandardCardPackage: public Package{
    Q_OBJECT

public:
    StandardCardPackage();
};

class StandardExCardPackage: public Package{
    Q_OBJECT

public:
    StandardExCardPackage();
};



#endif // STANDARDEQUIPS_H
