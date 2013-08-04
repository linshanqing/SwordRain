#ifndef GENERAL_H
#define GENERAL_H

class Skill;
class TriggerSkill;
class Package;
class QSize;

#include <QObject>
#include <QSet>
#include <QMap>
#include <QStringList>

class General : public QObject
{
    Q_OBJECT
    Q_ENUMS(Gender)
    Q_PROPERTY(QString kingdom READ getKingdom CONSTANT)
    Q_PROPERTY(int maxhp READ getMaxHp CONSTANT)
    Q_PROPERTY(bool male READ isMale STORED false CONSTANT)
    Q_PROPERTY(bool female READ isFemale STORED false CONSTANT)
    Q_PROPERTY(Gender gender READ getGender CONSTANT)
    Q_PROPERTY(bool lord READ isLord CONSTANT)
    Q_PROPERTY(bool hidden READ isHidden CONSTANT)

public:
    explicit General(Package *package, const QString &name, const QString &kingdom, int max_hp = 4, bool male = true, bool hidden = false, bool never_shown = false);

    // property getters/setters
    int getMaxHp() const; //获取体力上限
    QString getKingdom() const; //获取势力
    bool isMale() const; //是否为男性
    bool isFemale() const; //是否为女性
    bool isNeuter() const; //是否为中性
    bool isLord() const; //是否为主公
    bool isHidden() const; //是否隐藏(选将时不出现,武将一览中可见)
    bool isTotallyHidden() const; //是否永久隐藏(也不出现在武将一览中)

    enum Gender {SexLess, Male, Female, Neuter}; //性别枚举变量:无性别,男性,女性,中性
    Gender getGender() const; //获取性别--返回性别枚举变量Gender
    void setGender(Gender gender); //设置性别,参数为性别枚举变量

    void addSkill(Skill* skill);
    void addSkill(const QString &skill_name);    
    bool hasSkill(const QString &skill_name) const;
    QList<const Skill *> getSkillList() const;
    QList<const Skill *> getVisibleSkillList() const;
    QSet<const Skill *> getVisibleSkills() const;
    QSet<const TriggerSkill *> getTriggerSkills() const;

    void addRelateSkill(const QString &skill_name);
    QStringList getRelatedSkillNames() const;

    QString getPackage() const;
    QString getSkillDescription() const;

public slots:
    void lastWord() const;

private:
    QString kingdom;
    int max_hp;
    Gender gender;
    bool lord;
    QSet<QString> skill_set;
    QSet<QString> extra_set;
    QStringList related_skills;
    bool hidden;
    bool never_shown;
};

#endif // GENERAL_H
