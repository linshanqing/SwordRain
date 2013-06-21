-- translation for StandardPackage

local t = {
	["standard_cards"] = "标准卡牌包",
	["KongHou"] = "鬼柳",
	[":KongHou"] = "装备牌·武器\
攻击范围：3\
武器特效：当你使用的【杀】被目标角色的【闪】抵消时，你可以弃置其一张牌。",
	
	["MagicSword"] = "魔剑",
	[":MagicSword"] = "装备牌·武器\
攻击范围：2\
武器特效：当你使用【杀】指定一名角色为目标后，你可以进行一次判定，若判定结果为红色，该角色不可以使用【闪】对此【杀】进行响应。",

	["ice_slash"] = "冰杀",
	[":ice_slash"] = "冰杀",
	[":slash"] = "基本牌\
出牌时机：出牌阶段。\
使用目标：你攻击范围内的一名其他角色。\
作用效果：你对目标角色造成1点水属性伤害或弃置其两张牌。\
◆你在每个出牌阶段内只能使用一张【杀】。",
	["ice_nature"] = "水属性",
	
	["slash"] = "杀",
	[":slash"] = "基本牌\
出牌时机：出牌阶段。\
使用目标：你攻击范围内的一名其他角色。\
作用效果：你对目标角色造成1点伤害。\
◆你在每个出牌阶段内只能使用一张【杀】。",
	["slash_extra_targets:yes"] = "你可以为你的【杀】选择一个额外目标",
	["slash_extra_targets"] = "【杀】的额外目标",

	["jink"] = "闪",
	[":jink"] = "基本牌\
出牌时机：以你为目标的【杀】开始结算时\
使用目标：以你为目标的【杀】\
作用效果：抵消目标【杀】的效果",
	["#Slash"] = "%from 对 %to 使用了【杀】",
	["#Jink"] = "%from 使用了【闪】",

	["peach"] = "桃",
	[":peach"] = "基本牌\
出牌时机：1、出牌阶段。2、有角色处于濒死状态时。\
使用目标：1、已受伤的你。2、处于濒死状态的一名角色。\
作用效果：目标角色回复1点体力。",

	["Crossbow"] = "诸葛连弩",
	[":Crossbow"] = "装备牌·武器\
攻击范围：１\
武器特效：<b>锁定技</b>，你在出牌阶段内使用【杀】时无次数限制。",

	["DoubleSword"] = "雌雄双股剑",
	[":DoubleSword"] = "装备牌·武器\
攻击范围：２\
武器特效：当你使用【杀】指定一名异性角色为目标后，你可以令其选择一项：弃置一张手牌，或令你摸一张牌。",
	["double-sword-card"] = "%src 发动了雌雄双股剑特效，您必须弃置一张手牌或让 %src 摸一张牌",
	["DoubleSword:yes"] = "您可以让对方选择自弃置一牌或让您摸一张牌",

	["QinggangSword"] = "青釭剑",
	[":QinggangSword"] = "装备牌·武器\
攻击范围：２\
武器特效：<b>锁定技</b>，当你使用【杀】指定一名角色为目标后，你无视其防具。\
◆你使用【杀】指定A为目标后触发【青釭剑】无视A的防具，效果持续到该【杀】被【闪】抵消或A受到的伤害被防止或A受到的伤害进行伤害结算计算出最终的伤害值时为止。",

	["Blade"] = "青龙偃月刀",
	[":Blade"] = "装备牌·武器\
攻击范围：３\
武器特效：当你使用的【杀】被【闪】抵消时，你可以对相同的目标再使用一张【杀】（无距离限制）。",
	["blade-slash"] = "您可以对 %src 使用一张【杀】发动青龙偃月刀的追杀效果",
	["#BladeUse"] = "%from 使用了【青龙偃月刀】，对 %to 使用了一张【杀】",

	["Spear"] = "丈八蛇矛",
	[":Spear"] = "装备牌·武器\
攻击范围：３\
武器特效：你可以将两张手牌当【杀】使用或打出。",

	["Axe"] = "贯石斧",
	[":Axe"] = "装备牌·武器\
攻击范围：３\
武器特效：当你使用的【杀】被【闪】抵消时，你可以弃置两张牌，则此【杀】依然造成伤害。",
	["@Axe"] = "你可再弃置两张牌（包括装备）使此杀强制命中",
	["~Axe"] = "选择两张牌——点击确定",
	["#AxeSkill"] = "%from 使用了【%arg】的技能，弃置了2张牌以对 %to 强制命中",

	["Halberd"] = "方天画戟",
	[":Halberd"] = "装备牌·武器\
攻击范围：４\
武器特效：当你使用【杀】时，若此【杀】是你最后的手牌，你可以额外选择至多两个目标。",

	["KylinBow"] = "麒麟弓",
	[":KylinBow"] = "装备牌·武器\
攻击范围：５\
武器特效：每当你使用【杀】对目标角色造成伤害时，你可以弃置其装备区里的一张坐骑牌。",
	["KylinBow:yes"] = "弃置目标角色的一张坐骑牌",
	["KylinBow:dhorse"] = "【+1坐骑】",
	["KylinBow:ohorse"] = "【-1坐骑】",

	["EightDiagram"] = "八卦阵",
	[":EightDiagram"] = "装备牌·防具\
防具效果：当你需要使用或打出一张【闪】时，你可以进行一次判定，若判定结果为红色，则视为你使用或打出了一张【闪】。\
◆若判定结果为黑色，你仍可以用其他方式使用或打出一张【闪】。",
	["EightDiagram:yes"] = "进行一次判定，若判定结果为红色，则视为你打出了一张【闪】",

	["standard_ex_cards"] = "标准EX卡牌包",

	["RenwangShield"] = "仁王盾",
	[":RenwangShield"] = "装备牌·防具\
防具效果：<b>锁定技</b>，黑色的【杀】对你无效。",

	["IceSword"] = "寒冰剑",
	[":IceSword"] = "装备牌·武器\
攻击范围：２\
武器特效：每当你使用【杀】对目标角色造成伤害时，若该角色有牌，你可以防止此伤害，改为依次弃置其两张牌。",
	["IceSword:yes"] = "您可以弃置其两张牌",

	["Horse"] = "马",
	[":+1 Horse"] = "装备牌·坐骑\
坐骑效果：<b>锁定技</b>，其他角色计算的与你的距离+1。",
	["JueYing"] = "绝影",
	["DiLu"] = "的卢",
	["ZhuaHuangFeiDian"] = "爪黄飞电",
	[":-1 Horse"] = "装备牌·坐骑\
坐骑效果：<b>锁定技</b>，你计算的与其他角色的距离-1。",
	["ChiTu"] = "赤兔",
	["DaYuan"] = "大宛",
	["ZiXing"] = "紫骍",

	["amazing_grace"] = "五谷丰登",
	[":amazing_grace"] = "锦囊牌\
出牌时机：出牌阶段。\
使用目标：所有角色。\
执行动作：（选择目标后）你从牌堆顶亮出等同于现存角色数量的牌。\
作用效果：每名角色选择并获得这些牌中的一张。",

	["god_salvation"] = "桃园结义",
	[":god_salvation"] = "锦囊牌\
出牌时机：出牌阶段。\
使用目标：所有角色。\
作用效果：每名角色回复1点体力。\
◆【桃园结义】对未受伤的角色无效。",

	["savage_assault"] = "南蛮入侵",
	[":savage_assault"] = "锦囊牌\
出牌时机：出牌阶段。\
使用目标：所有其他角色。\
作用效果：每名目标角色需打出一张【杀】，否则你对其1点伤害。",
	["savage-assault-slash"] = "%src 使用了【南蛮入侵】，请打出一张【杀】进行响应",

	["archery_attack"] = "万箭齐发",
	[":archery_attack"] = "锦囊牌\
出牌时机：出牌阶段。\
使用目标：所有其他角色。\
作用效果：每名目标角色需打出一张【闪】，否则你对其造成1点伤害。",
	["archery-attack-jink"] = "%src 使用了【万箭齐发】，请打出一张【闪】进行响应",

	["collateral"] = "借刀杀人",
	[":collateral"] = "锦囊牌\
出牌时机：出牌阶段。\
使用目标：装备区里有武器牌的一名其他角色A。\
执行动作：（选择目标时）你选择A攻击范围内的另一名角色B。\
作用效果：A需对B使用一张【杀】，否则A必须将其装备区里的武器牌交给你。\
◆B可以为你。\
◆使用【借刀杀人】的过程中须进行两次使用【杀】的合法性检测，第一次是在A选择B使用【杀】的目标时（此时【杀】并未置入处理区，B只是被选择为将要使用的【杀】的目标），因此不可以对攻击范围内没有合法目标的角色使用【借刀杀人】，第二次是在B对C使用【杀】时。",
	["collateral-slash"] = "%src 使用了【借刀杀人】，令你砍 %dest，请你使用一张【杀】进行响应",
	["#CollateralSlash"] = "%from 选择了此【杀】的目标 %to",

	["duel"] = "决斗",
	[":duel"] = "锦囊牌\
出牌时机：出牌阶段。\
使用目标：一名其他角色。\
作用效果：由目标角色开始，你与其轮流打出一张【杀】，首先不出【杀】的一方受到另一方造成的1点伤害。",
	["duel-slash"] = "%src 对你使用【决斗】，请你打出一张【杀】进行响应",

	["ex_nihilo"] = "无中生有",
	[":ex_nihilo"] = "锦囊牌\
出牌时机：出牌阶段。\
使用目标：你。\
作用效果：你摸两张牌。",

	["snatch"] = "顺手牵羊",
	[":snatch"] = "锦囊牌\
出牌时机：出牌阶段。\
使用目标：距离为1且区域里有牌的一名其他角色。\
作用效果：你获得其区域里的一张牌。",

	["dismantlement"] = "过河拆桥",
	[":dismantlement"] = "锦囊牌\
出牌时机：出牌阶段。\
使用目标：区域里有牌的一名其他角色。\
作用效果：你将其区域里的一张牌弃置。",

	["nullification"] = "无懈可击",
	[":nullification"] = "锦囊牌\
出牌时机：目标锦囊牌生效前。\
使用目标：一张锦囊。\
作用效果：抵消目标锦囊牌对一名角色产生的效果，或抵消另一张【无懈可击】产生的效果。",

	["indulgence"] = "乐不思蜀",
	[":indulgence"] = "延时锦囊牌\
出牌时机：出牌阶段。\
使用目标：一名其他角色。\
执行动作：你将【乐不思蜀】置于目标角色的判定区里。\
作用效果：若判定结果不为红桃，跳过目标角色的出牌阶段；若判定结果为红桃，则没有事发生。",

	["lightning"] = "闪电",
	[":lightning"] = "延时锦囊牌\
出牌时机：出牌阶段。\
使用目标：你。\
执行动作：你将【闪电】置于你的判定区里。\
作用效果：若判定结果为黑桃2-9，则目标角色受到3点雷电伤害；若判定不为此结果，将之移动到下家的判定区里。\
◆【闪电】的目标可能会不断地进行改变，直到它被置入弃牌堆。当【无懈可击】抵消角色判定区里的【闪电】的效果后，【闪电】不会被置入弃牌堆，而是将它按逆时针顺序从处理区移至下一名合法目标的判定区。如果所有角色都不是合法的目标，【闪电】无法更换目标，继续置于该角色的判定区里。\
◆【闪电】造成的伤害没有来源。",

}

local ohorses = {"ChiTu", "DaYuan", "ZiXing"}
local dhorses = {"ZhuaHuangFeiDian", "DiLu", "JueYing", "HuaLiu"}

for _, horse in ipairs(ohorses) do
	t[":" .. horse] = t[":-1 Horse"]
end

for _, horse in ipairs(dhorses) do
	t[":" .. horse] = t[":+1 Horse"]
end

return t
