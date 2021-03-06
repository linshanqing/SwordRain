#include <exppattern.h>

ExpPattern::ExpPattern(const QString &exp) {
    this->exp = exp;
}

bool ExpPattern::match(const Player *player, const Card *card) const{
    foreach (QString one_exp, this->exp.split('#'))
        if (this->matchOne(player, card, one_exp)) return true;

    return false;
}

// '|' means 'and', '#' means 'or'.
// the expression splited by '#' has 3 parts,
// 1st part means the card name, and ',' means more than one options.
// 2nd patt means the card suit, and ',' means more than one options.
// 3rd part means the card number, and ',' means more than one options,
// the number uses '~' to make a scale for valid expressions
bool ExpPattern::matchOne(const Player *player, const Card *card, QString exp) const{
    QStringList factors = exp.split('|');

    bool checkpoint = false;
    QStringList card_types = factors.at(0).split(',');
    foreach (QString or_name, card_types) {
        checkpoint = false;
        foreach (QString name, or_name.split('+')) {
            if (name == ".") {
                checkpoint = true;
            } else {
                bool isInt = false;
                bool positive = true;
                if (name.startsWith('^')) {
                    positive = false;
                    name = name.mid(1);
                }
                if (card->isKindOf(name.toLocal8Bit().data()) || (card->getEffectiveId() == name.toInt(&isInt) && isInt))
                    checkpoint = positive;
                else
                    checkpoint = !positive;
            }
            if (!checkpoint) break;
        }
        if (checkpoint) break;
    }
    if (!checkpoint) return false;
    if (factors.size() < 2) return true;

    checkpoint = false;
    QStringList card_suits = factors.at(1).split(',');
    foreach(QString suit, card_suits) {
        if (suit == ".") {checkpoint = true; break;}
        bool positive = true;
        if (suit.startsWith('^')) {
            positive = false;
            suit = suit.mid(1);
        }
        if (card->getSuitString() == suit)
            checkpoint = positive;
        else
            checkpoint = !positive;
        if (checkpoint) break;
    }
    if (!checkpoint) return false;
    if (factors.size() < 3) return true;

    checkpoint = false;
    QStringList card_numbers = factors.at(2).split(',');
    int cdn = card->getNumber();

    foreach (QString number, card_numbers) {
        if (number == ".") {checkpoint = true; break;}
        if (number.contains('~')) {
            QStringList params = number.split('~');
            int from, to;
            if (!params.at(0).size())
                from = 1;
            else
                from = params.at(0).toInt();
            if (!params.at(1).size())
                to = 13;
            else
                to =params.at(1).toInt();

            if (from <= cdn && cdn <= to) checkpoint = true;
        }else if (number.toInt() == cdn)
            checkpoint = true;
        if (checkpoint) break;
    }
    if (!checkpoint) return false;
    if (factors.size() < 4) return true;

    checkpoint = false;
    QString place = factors.at(3);
    if (place == ".") checkpoint = true;
    else if (place == "equipped" && player->hasEquip(card)) checkpoint = true;
    else if (place == "hand" && card->getEffectiveId() >= 0 && !player->hasEquip(card)) checkpoint = true;
    if (!checkpoint) return false;
    if (factors.size() < 5) return true;

    QString color = factors.at(4);
    if (color == ".") return true;
    else if (color == "red" && card->isRed()) return true;
    else if (color == "black" && card->isBlack()) return true;
    else if (color == "colorless" && card->getSuit() == Card::NoSuit) return true;

    return false;
}

