; NSIS script NSIS-2
; Install

SetCompressor lzma
SetCompressorDictSize 8

; --------------------
; HEADER SIZE: 78085
; START HEADER SIZE: 300
; MAX STRING LENGTH: 1024
; STRING CHARS: 18295

OutFile [NSIS].exe
!include WinMessages.nsh



; --------------------
; LANG TABLES: 1
; LANG STRINGS: 51

Name 哒哒网游加速器
BrandingText "Nullsoft Install System v2.46"

; LANG: 2052
LangString LSTR_0 2052 "Nullsoft Install System v2.46"
LangString LSTR_1 2052 "$(LSTR_2) 安装"
LangString LSTR_2 2052 哒哒网游加速器
LangString LSTR_5 2052 "无法写入: "
LangString LSTR_6 2052 "复制失败 "
LangString LSTR_7 2052 "复制到: "
LangString LSTR_8 2052 "无法找到符号: "
LangString LSTR_9 2052 "无法加载: "
LangString LSTR_10 2052 "创建文件夹: "
LangString LSTR_11 2052 "创建快捷方式: "
LangString LSTR_13 2052 "删除文件: "
LangString LSTR_14 2052 "重新启动后删除: "
LangString LSTR_15 2052 "正在创建快捷方式时发生错误: "
LangString LSTR_17 2052 正在解压缩数据发生错误！已损坏的安装程序？
LangString LSTR_19 2052 "运行外部程序: "
LangString LSTR_20 2052 "运行: "
LangString LSTR_21 2052 "抽取: "
LangString LSTR_22 2052 "抽取: 无法写入文件 "
LangString LSTR_23 2052 "安装损毁: 无效的操作代码 "
LangString LSTR_24 2052 "没有 OLE 用于: "
LangString LSTR_25 2052 "输出目录: "
LangString LSTR_26 2052 "移除目录: "
LangString LSTR_27 2052 "重新启动后重命名: "
LangString LSTR_28 2052 "重命名: "
LangString LSTR_29 2052 "已跳过: "
LangString LSTR_30 2052 "复制细节到剪贴板 "
LangString LSTR_36 2052 宋体
LangString LSTR_37 2052 9
LangString LSTR_38 2052 "不能打开要写入的文件: $\r$\n$\t$\"$0$\"$\r$\n单击 [Abort] 放弃安装，$\r$\n [Retry] 重新尝试写入文件，或$\r$\n [Ignore] 忽略这个文件。"
LangString LSTR_39 2052 自定义
LangString LSTR_40 2052 取消(&C)
LangString LSTR_41 2052 "< 上一步(&B)"
LangString LSTR_42 2052 安装(&I)
LangString LSTR_43 2052 "单击 [安装(I)] 开始安装进程。"
LangString LSTR_44 2052 ": 正在安装"
LangString LSTR_45 2052 显示细节(&D)
LangString LSTR_46 2052 已完成
LangString LSTR_47 2052 "下一步(&N) >"
LangString LSTR_48 2052 "单击 [下一步(N)] 继续。"
LangString LSTR_49 2052 ": 已完成"
LangString LSTR_50 2052 关闭(&L)


; --------------------
; VARIABLES: 22

Var _0_
Var _1_
Var _2_
Var _3_
Var _4_
Var _5_
Var _6_
Var _7_
Var _8_
Var _9_
Var _10_
Var _11_
Var _12_
Var _13_
Var _14_
Var _15_
Var _16_
Var _17_
Var _18_
Var _19_
Var _20_
Var _21_


InstType $(LSTR_39)    ;  自定义
InstallDir $PROGRAMFILES\DaDaJiaSu
; install_directory_auto_append = DaDaJiaSu
; wininit = $WINDIR\wininit.ini


; --------------------
; PAGES: 3

; Page 0
Page custom func_1102 "" /ENABLECANCEL

; Page 1
Page instfiles "" func_1295 ""
  CompletedText $(LSTR_46)    ;  已完成
  DetailsButtonText $(LSTR_45)    ;  显示细节(&D)

/*
; Page 2
Page COMPLETED
*/


; --------------------
; SECTIONS: 1
; COMMANDS: 2071

Function .onGUIInit
  GetDlgItem $_0_ $HWNDPARENT 1037
  CreateFont $_1_ $(LSTR_36) $(LSTR_37) 700    ;  宋体 9
  SendMessage $_0_ ${WM_SETFONT} $_1_ 0
  GetDlgItem $_2_ $HWNDPARENT 1038
  SetCtlColors $_0_ "" 0xFFFFFF
  SetCtlColors $_2_ "" 0xFFFFFF
  GetDlgItem $_3_ $HWNDPARENT 1034
  SetCtlColors $_3_ "" 0xFFFFFF
  GetDlgItem $_4_ $HWNDPARENT 1039
  SetCtlColors $_4_ "" 0xFFFFFF
  GetDlgItem $_6_ $HWNDPARENT 1028
  SetCtlColors $_6_ /BRANDING ""
  GetDlgItem $_5_ $HWNDPARENT 1256
  SetCtlColors $_5_ /BRANDING ""
  SendMessage $_5_ ${WM_SETTEXT} 0 "STR:$(LSTR_0) "    ;  "Nullsoft Install System v2.46"
  GetDlgItem $_7_ $HWNDPARENT 1035
  GetDlgItem $_8_ $HWNDPARENT 1045
  GetDlgItem $_9_ $HWNDPARENT 1
  GetDlgItem $_10_ $HWNDPARENT 2
  GetDlgItem $_11_ $HWNDPARENT 3
FunctionEnd


Function .onUserAbort
FunctionEnd


Section MainSetup ; Section_0
  ; AddSize 39009
  Call func_2035
  Call func_1973
  Call func_1552
  SetOutPath $INSTDIR
  StrCpy $_OUTDIR $OUTDIR
  SetOverwrite try
  File 1.txt
  File CommonLib.dll
  File CrashReport.exe
  File DDDetour.dll
  File DDDetourHelper_x64.bin
  File DDDetour_x64.dll
  File DDIP.dll
  File DDIP64.dll
  File DDNetwork.dll
  File DDTrafficMonitor.dll
  File DDTun2Proxy.dll
  File DDURL.dll
  File DaDaJiaSu.exe
  File DuiLib.dll
  File Game_Info.txt
  File LSPHelper.bin
  File LSPHelper64.bin
  File MetaSpd.dll
  File dadaGameAnalyse.exe
  File dadaPcMonitor.exe
  File dadaTray.exe
  File dadaupdate.dll
  File hosts_unlock.bat
  File libeay32.dll
  File msvcp100.dll
  File msvcr100.dll
  File msvcr120.dll
  File uninst.exe
  File wke.dll
  File 修复不能上网.bat
  SetOutPath $_OUTDIR\50
  File DDTrafficMonitor_x64.sys
  File DDTrafficMonitor_x86.sys
  SetOutPath $_OUTDIR\60
  File DDTrafficMonitor_x64.sys
  File DDTrafficMonitor_x86.sys
  SetOutPath $_OUTDIR\CacheBuf
  File allGames.cfg
  File allGamesTag.cfg
  File allProxys.cfg
  SetOutPath $_OUTDIR\GameBkImg
  SetOutPath $_OUTDIR\GameIcon
  File 300英雄.png
  File ACEonline（美服）.png
  File AVA战地之王.png
  File AVA战地之王（韩服）.png
  File AVA战地之王（台服）.png
  File "Atlas Reactor.png"
  File Battleborn-steam.png
  File "Black Squad（Steam）.png"
  File "Block N Load.png"
  File Brawlhalla-Steam.png
  File CSGO-Steam.png
  File CSGO（国服）.png
  File CSOnline（台服）.png
  File "Cloud Pirates.png"
  File Creativerse-Steam.png
  File D10.png
  File DayZ-Steam.png
  File Deformers(测试).png
  File Dota2.png
  File Dota2（外服）-steam.png
  File ELOA(艾洛亚online).png
  File EVE-Online.png
  File EVE-Online（外服）.png
  File "FIFA online 3.png"
  File "Fistful of Frags（群战）.png"
  File Formicide（除蚁剂）.png
  File GTA5-steam(国内组队).png
  File GTA5-steam.png
  File GT劲舞团2.png
  File "Gang Beasts（基佬大乱斗）-steam.png"
  File GloriaVictis（征服的荣耀）-Steam.png
  File Gmod.png
  File H1Z1（北美）.png
  File H1Z1（生存模式）.png
  File H1Z1（亚太）.png
  File Hawken.png
  File "Human Fall Flat-Steam.png"
  File Hurtworld（伤害世界）.png
  File "Last Man Standing-Steam.png"
  File MarvelHeroesOmega.png
  File Meadow.png
  File Miscreated-Steam.png
  File NBA2K17（测试中）.png
  File NBA2K18（测试中）.png
  File NBA2KOnline.png
  File NBA2KOnline（台服）.png
  File "Next Day-Survival.png"
  File "OldSchool RuneScape.png"
  File Onraid-Steam.png
  File QQ飞车.png
  File QQ华夏.png
  File QQ幻想世界.png
  File QQ降龙.png
  File QQ三国.png
  File QQ堂.png
  File QQ西游.png
  File QQ音速.png
  File QQ炫舞.png
  File QQ炫舞2.png
  File R2-台服.png
  File R2.png
  File Raiders.png
  File Robocraft-Steam.png
  File RuneScape.png
  File Rust腐蚀-steam.png
  File SOS终极大逃杀.png
  File Skyforge.png
  File Squad-steam.png
  File "Star Conflict.png"
  File TROVE.png
  File Tera（台服测试）.png
  File "The Isle.png"
  File "Ultima Onlina（UO）.png"
  File "alien Swarm（异星虫群）.png"
  File argo.png
  File default.png
  File gta5（内部测试）.png
  File python.png
  File steam商城社区.png
  File steep-steam.png
  File warframe-steam.png
  File 艾尔之光.png
  File 暗黑破坏神3.png
  File 暗黑破坏神3（美服）.png
  File 暗黑破坏神3（台服）.png
  File 百变兵团（台服）.png
  File 堡垒之夜-国服.png
  File 堡垒之夜-国际服.png
  File 变形金刚OL.png
  File 彩虹岛.png
  File 彩虹岛（台服）.png
  File 彩虹六号-围攻-Uplay.png
  File 彩虹六号-围攻-游侠.png
  File 测试测试.png
  File 测试测试2.png
  File 测试游戏.png
  File 超凡战纪.png
  File 超级鸡马-Steam.png
  File 超神英雄（美服）.png
  File 尘埃4.png
  File 成吉思汗3.png
  File 赤壁.png
  File 穿越火线.png
  File 传奇3.png
  File 传奇3（韩服）.png
  File 传奇世界-盛大.png
  File 传奇世界2.png
  File 传奇外传.png
  File 传奇永恒.png
  File 创世战车（外服）.png
  File 创想兵团.png
  File 春秋Q传.png
  File 大冲锋.png
  File 大航海时代OL（台服）.png
  File 大话西游2经典版.png
  File 大话西游2免费版.png
  File 大明龙权.png
  File 大唐无双零.png
  File 刀锋铁骑.png
  File 刀剑2.png
  File 刀剑英雄.png
  File 盗贼之海.png
  File 地城之光.png
  File "地牢守护者2（Dungeon Defenders II）.png"
  File 地下城与勇士.png
  File 地狱潜者（helldivers）.png
  File 第九大陆-C9.png
  File 帝国时代2HD.png
  File "钓鱼星球（Fishing Planet）.png"
  File 斗仙.png
  File 反恐精英Online.png
  File 反恐精英Online2.png
  File 反恐行动.png
  File 方块方舟（PiXARK）.png
  File 方舟-大逃杀.png
  File 方舟-生存进化（国服）.png
  File 方舟-生存进化（美服）.png
  File 方舟-生存进化（欧服）.png
  File 废品机械师-steam.png
  File 风暴英雄.png
  File 风暴英雄（台服）.png
  File 风云之通天之路.png
  File 凤舞天骄.png
  File 钢铁之师-诺曼底44-steam.png
  File 孤岛惊魂5.png
  File 怪物猎人Online.png
  File 光荣使命.png
  File 光之国度（美服）.png
  File 海战世界.png
  File 海之乐章-启航.png
  File "黑暗与光明（Dark and Light）.png"
  File 黑暗之魂3-steam.png
  File 黑色沙漠（台服）.png
  File 黑色阴谋2（台服）.png
  File "红龙（wargame red dragon）.png"
  File 虎豹骑-Steam.png
  File 虎豹骑（国服）.png
  File 花园战争2.png
  File 华夏2.png
  File 画江山.png
  File 幻想大陆.png
  File 幻想大陆（日服）.png
  File 幻想联盟.png
  File 幻想全明星.png
  File 幻想神域.png
  File 幻想神域（美服）.png
  File 幻想学园.png
  File 荒野行动-PC.png
  File 毁灭战士4（DOOM）.png
  File 火箭联盟-Steam.png
  File 火炬之光2-游侠.png
  File 火影忍者.png
  File 机动战士敢达OL(国服).png
  File 机战.png
  File 饥荒-TGP.png
  File 饥荒-steam.png
  File 饥荒-游侠.png
  File 激战2.png
  File 激战2（美服）.png
  File 极光世界武帝降临.png
  File 极品飞车14(亚服）.png
  File 极品飞车17(亚服）.png
  File 极品飞车19(亚服）.png
  File 极品飞车20（亚服）.png
  File 极品飞车ONLINE.png
  File 疾风之刃.png
  File 疾风之刃（台服）.png
  File 剑灵.png
  File 剑灵（韩服）.png
  File 剑灵（台服）.png
  File 剑侠情缘2.png
  File 剑侠情缘3免费版（台服）.png
  File 剑侠情缘网络版叁（重制版）.png
  File 剑侠世界2.png
  File 街头霸王5.png
  File 街头篮球.png
  File 金庸群侠传怀旧版.png
  File 劲舞团.png
  File 精灵传说.png
  File 精灵复兴.png
  File 竞技场(测试).png
  File 九阳神功.png
  File 九阴真经.png
  File 九阴真经（北美版）.png
  File 九阴真经（台服）.png
  File 救世之树(台服).png
  File 救世之树-steam.png
  File 救赎2online.png
  File 巨人.png
  File 绝地求生(澳服).png
  File 绝地求生(东南亚).png
  File 绝地求生(韩服).png
  File 绝地求生(美服).png
  File 绝地求生(欧服).png
  File 绝地求生(日服).png
  File 绝地求生(网吧测试).png
  File 绝地求生(亚太).png
  File 绝地求生（全球服）.png
  File 绝地求生测试（勿用）.png
  File 绝地求生刺激战场.png
  File 绝地求生全军出击.png
  File 军团要塞2-Steam.png
  File 看门狗2.png
  File 昆特牌.png
  File 昆特牌（国服）.png
  File 黎明杀机-steam.png
  File 黎明杀机-游侠.png
  File 猎人：野性的召唤.png
  File 猎杀对决.png
  File 灵魂武器（日服）.png
  File 领地人生-steam.png
  File 流放者柯南-steam.png
  File 流亡之路（国服）.png
  File 流亡黯道（国际服）.png
  File 流亡黯道（台服）.png
  File 流亡黯道（新加坡专服）.png
  File 龙魂时刻.png
  File 龙腾世界.png
  File 龙武2.png
  File 龙之谷.png
  File 炉石传说.png
  File 炉石战记（台服）.png
  File 绿色征途（测试版）.png
  File 洛奇.png
  File 洛奇英雄传.png
  File 冒险岛.png
  File 冒险岛2.png
  File 萌兽学院.png
  File 梦幻国度.png
  File 梦幻龙族（台服）.png
  File 梦幻西游2.png
  File 梦幻诛仙2.png
  File 梦境OL（台服）.png
  File 梦三国.png
  File 梦三国2.png
  File 梦塔防.png
  File 梦想世界.png
  File 梦想世界2.png
  File 模拟农场17.png
  File 魔力宝贝.png
  File 魔兽世界.png
  File 魔兽世界（美服）.png
  File 魔兽世界（欧服）.png
  File 魔兽世界（台服）.png
  File 魔侠传.png
  File 魔域（美服）.png
  File 魔域怀旧版.png
  File 魔域正式版.png
  File 逆战.png
  File 诺亚传说.png
  File 欧洲卡车模拟2.png
  File 跑跑卡丁车.png
  File 七日杀-游侠.png
  File 七日杀（7DaysToDie）-Steam.png
  File 奇迹MU.png
  File 奇迹世界SUM2.png
  File 奇迹世界（韩服）.png
  File 骑马与砍杀.png
  File 骑士中世纪战争.png
  File 千年3.png
  File 枪火游侠-国服（内测）.png
  File 枪火游侠（美服）.png
  File 枪神纪.png
  File 晴空物语（台服）.png
  File 全境封锁-Uplay.png
  File 全球使命2.png
  File 全职大师（美服）.png
  File 热血传奇.png
  File 热血江湖.png
  File 忍者村大战2.png
  File 荣耀战魂-Uplay.png
  File 三国策.png
  File 三国群英传OL.png
  File 散兵坑（Foxhole）.png
  File "森林（The Forest）-Steam.png"
  File "森林（The Forest）-游侠.png"
  File 杀戮空间2-steam.png
  File 伤害世界-游侠.png
  File 上古世纪.png
  File 上古世纪（台服）.png
  File 赦免者（Absolver）.png
  File 深海惊魂（Depth）-Steam.png
  File 神鬼传奇.png
  File 神鬼世界.png
  File 神话2.png
  File 神秘世界传奇（测试）.png
  File 神泣（美服）.png
  File 神武.png
  File 神战奇迹.png
  File 神之浩劫.png
  File 神之浩劫（外服）.png
  File 生存指南2.png
  File "生命之树（Tree of Life）.png"
  File 盛世征途.png
  File 圣境传说（台服）.png
  File 十二之天2经典版.png
  File 十三号星期五.png
  File 实况足球2018.png
  File 使命召唤12-Steam（测试中）.png
  File 使命召唤13-Steam.png
  File 使命召唤14-steam.png
  File 使命召唤9-Steam.png
  File 使命召唤OL.png
  File 收获日2.png
  File 收获日2（Payday2).png
  File 守望先锋.png
  File 守望先锋（美服）.png
  File 守望先锋（欧服）.png
  File 守望先锋（亚服）.png
  File 兽人必须死.png
  File 蜀山缥缈录.png
  File 蜀山缥缈录（台服）.png
  File 数码暴龙online.png
  File 泰坦陨落2.png
  File 泰亚史诗.png
  File 坦克部队-Steam.png
  File 坦克大战-腾讯游戏.png
  File 坦克世界.png
  File 坦克世界（美服）.png
  File 坦克世界（亚服）.png
  File 桃花源记.png
  File 桃花源记2之御剑红尘.png
  File 特种部队OL.png
  File "天际起源（Horizon Source）.png"
  File 天炼.png
  File 天命2（命运2）.png
  File 天堂.png
  File 天堂2（台服）.png
  File 天涯明月刀.png
  File 天翼决.png
  File 天之禁.png
  File 天谕.png
  File 天谕（美服）.png
  File 跳伞求生.png
  File 铁甲雄兵（国服）.png
  File 铁拳7.png
  File 投名状.png
  File 吞噬苍穹.png
  File 完美世界国际版2.png
  File 王牌对决.png
  File 王牌对决（台服）.png
  File 王者荣耀（腾讯手游助手）.png
  File 未转变者（Unturned）-steam.png
  File 问道.png
  File 我的世界中国版.png
  File 无尽战区.png
  File 无上神兵.png
  File 无限法则（外服）.png
  File 无主之地2-steam.png
  File 武魂2.png
  File 武林外传.png
  File 武神.png
  File 武圣.png
  File 希望online.png
  File 侠义道.png
  File 侠义道Ⅱ.png
  File 笑傲江湖OL.png
  File 新龙之谷（台服）.png
  File 新玛奇英雄传（台服）.png
  File 新墨香.png
  File 新蜀门.png
  File 新蜀山OL.png
  File 新丝路.png
  File 新天骄.png
  File 新天龙八部.png
  File 新天龙八部（港服）.png
  File 新挑战.png
  File 新侠义道.png
  File 新寻仙.png
  File 新倚天.png
  File 新英雄.png
  File 新征途.png
  File 新倩女幽魂.png
  File 星际战甲.png
  File 星际争霸2.png
  File 星际争霸2（外服）.png
  File 星球大战：前线2（测试）.png
  File 星球大战前线（美服）.png
  File 行星边际2（美服）.png
  File 虚幻争霸（美服）.png
  File 轩辕传奇.png
  File 英魂之刃.png
  File 英魂之刃（美服）.png
  File 英魂之刃（台服）.png
  File 英雄联盟.png
  File 英雄联盟（韩服）.png
  File 英雄联盟（美服）.png
  File 英雄联盟（日服）.png
  File 英雄联盟（台服）.png
  File 英雄联盟（新加坡）.png
  File 英雄三国.png
  File 英雄与将军-steam.png
  File 影武者.png
  File 影之诗（Shadowverse）-Steam.png
  File 永恒之塔.png
  File 幽灵行动-荒野-Uplay.png
  File 御剑红尘.png
  File 远征Online.png
  File 脏弹.png
  File 战锤40k-战争黎明3.png
  File 战锤末日鼠疫2.png
  File 战地1.png
  File 战地1（全球服）.png
  File 战地4-（全球服）.png
  File 战地风云-硬仗.png
  File 战舰世界.png
  File 战舰世界（美服）.png
  File 战舰世界（亚服）.png
  File 战狼online（台服）.png
  File 战争机器4（美服）.png
  File 战争雷霆.png
  File 战争雷霆（美服）.png
  File 战争前线（港服）.png
  File 战争前线（国服）.png
  File 战争前线（美服）.png
  File 战争前线（欧服）.png
  File 战争仪式（Battlerite）-steam.png
  File 战争艺术-赤潮.png
  File "战争召唤（Call to Arms）.png"
  File 征服.png
  File 征途.png
  File 征途2.png
  File 植物大战僵尸花园战争-origin.png
  File 终极火力.png
  File 终结者2审判日-PC.png
  File 重火力：起源-Steam.png
  File 诛仙2.png
  File 诛仙3.png
  File 装甲战争（国服）.png
  File 装甲战争（美服）.png
  File 自由篮球.png
  File 自由足球.png
  File 足球经理OL.png
  File 醉逍遥之乾坤一剑.png
  File 最后一炮.png
  File 最终幻想14.png
  File 喵喵喵.png
  File 逍遥江湖.png
  File 飚车世界.png
  File 炫舞吧.png
  SetOutPath $_OUTDIR\HotGameIcon
  File 300英雄.png
  File CSGO-STEAM.png
  File DOTA2.png
  File GTA5-STEAM(国内组队).png
  File GTA5-STEAM.png
  File H1Z1（亚太）.png
  File SOS终极大逃杀.png
  File STEAM商城社区.png
  File 堡垒之夜-国服.png
  File 堡垒之夜-国际服.png
  File 彩虹六号-围攻-UPLAY.png
  File 穿越火线.png
  File 地下城与勇士.png
  File 方块方舟（PIXARK）.png
  File 剑灵.png
  File 剑侠情缘网络版叁（重制版）.png
  File 绝地求生(东南亚).png
  File 绝地求生(美服).png
  File 绝地求生(亚太).png
  File 绝地求生（全球服）.png
  File 绝地求生刺激战场.png
  File 猎杀对决.png
  File 魔兽世界.png
  File 守望先锋.png
  File "天际起源（HORIZON SOURCE）.png"
  File 无限法则（外服）.png
  File 英雄联盟.png
  File 英雄联盟（韩服）.png
  File 战锤末日鼠疫2.png
  SetOutPath $_OUTDIR\Logs
  SetOutPath $_OUTDIR\WebErrorLog
  SetOutPath $_OUTDIR\cfgs
  File DBConfigure.ini
  File MyGameConfigure.xml
  File cfgs.ini
  File cfgs_1.ini
  File games.xml
  File msgboxinfo.xml
  File mygame.ini
  File mygame.xml
  File mygame_1.ini
  SetOutPath $_OUTDIR\localLogs
  SetOutPath $_OUTDIR\skin
  File AccGameMainWnd.xml
  File AccLogBk1.png
  File AccLogBk2.png
  File AccedOnBtn.png
  File AddDuanItem.png
  File AllGamesMainWnd.xml
  File AllGamesSubHeadWnd.xml
  File AllGamesSubWnd.xml
  File BackMainWnd.png
  File BackMainWorld.png
  File Bkimg350_360.png
  File BtnStartSetPath.png
  File Cancel110_36.png
  File CopyAccLog.png
  File DYCodeEditLog.png
  File DadaAbort.png
  File DadaExit.png
  File Default.xml
  File DeletItem.png
  File EmailEditLog.png
  File ErrorLog.png
  File ExitLogin_70_22.png
  File FeedBack.png
  File FindGameItem.png
  File FindGameNull.png
  File FindGameOk.png
  File FindGameOk140_40.png
  File FindGameStopScan.png
  File FindGameStopScan140_40.png
  File FloatCharge.png
  File FloatCharge2.png
  File FloatUp.png
  File FlowMonitor.png
  File GameBtnC150_36.png
  File GameBtnC150_36.png.png
  File GameBtnL150_36.png
  File GameBtnR150_36.png
  File GameBtnR150_36.png.png
  File GameHotItem.png
  File GameItemWnd.xml
  File GamePingtaiItem.png
  File HeaderOptBk.png
  File HourBEdit.png
  File HourLogin.png
  File HourSEdit.png
  File Kaitonghuiyuan.png
  File Loginlog.png
  File MainWorldBtn.png
  File Main_Charge.png
  File Main_Charge2.png
  File Main_GameWorld.png
  File Main_HostNotify.png
  File Main_Qustion.png
  File Masktool.png
  File Ok110_36.png
  File OptSpaceIcon.png
  File PasswordEditLog.png
  File QQ27.png
  File QQ27Min.png
  File QQBtnLog.png
  File QQBtnLog1.png
  File QQForCafes.png
  File RedCharge.png
  File RedClose.png
  File RedTab1Bk.png
  File RedTab2Bk.png
  File ResetLSP.png
  File RightMainBk.png
  File SelGamePath.png
  File SelectAcc.png
  File SelectLog.png
  File SelectNodeWnd.xml
  File SetPathLog.png
  File ShowMain.png
  File SortType.png
  File StartSet.png
  File StopAccBtn.png
  File SystemSet.png
  File UserInfoBk_Level.png
  File UserInfoBk_Nor.png
  File UserInfoLevelLog.png
  File UserInfoLog.png
  File UserLevelLog.png
  File WndBindPhone.xml
  File WndChargeForHour.xml
  File WndChargeHourBk.png
  File WndChargeOpt.png
  File WndChargeTip.xml
  File WndCheckDayBox.xml
  File WndDYCodeCheck.xml
  File WndDadaAbort.xml
  File WndDadaActive.xml
  File WndDadaMsg.xml
  File WndDelGameBox.xml
  File WndFindGames.xml
  File WndFindGamesAllFile.xml
  File WndFloatUserTime.xml
  File WndForNorLevel.xml
  File WndGameWorldInfo.xml
  File WndIEForLoginOrCharge.xml
  File WndLoginOrRegist.xml
  File WndMsgBox.xml
  File WndNetTool.xml
  File WndPingJia.xml
  File WndRedGet.xml
  File WndReportWorld.xml
  File WndRightMainWnd.xml
  File WndSelectWorld.xml
  File WndSetGamePath.xml
  File WndSetNetType.xml
  File WndSkinSet.xml
  File WndStopAcc.xml
  File WndSystemSet.xml
  File WndUpdate.xml
  File WndUserInfo.xml
  File WndWaitBox.xml
  File WndWaitForConnect.xml
  File WndWkeForLoginOrCharge.xml
  File accedinfolog.png
  File accingLogBk.gif
  File accingbtnicon.png
  File accingscore.png
  File accmodeopt.png
  File accmodewndbk.png
  File acitvityHeaderBk.png
  File arrowb.png
  File arrowg.png
  File autoselNode.png
  File bkimg.png
  File bkimg180_60.png
  File bkimg350_440.png
  File btnRedGet.png
  File btnbindPhone.png
  File btntuiguan.png
  File bulb.png
  File check130-30.png
  File checkbox1.png
  File checkinBk.png
  File close1.png
  File combo110_32.png
  File connectBk.png
  File curpathbk.png
  File dadaAbortBk.png
  File dadaLoading.gif
  File dadaTitleSet.png
  File dadaTitleZoon.png
  File dadajiasBaijin.gif
  File dadajiasHuangguan.gif
  File dadajiasNormal.gif
  File dadajiasZauanshi.gif
  File dadajiasuMain.xml
  File dadajiasu_Warn.png
  File dadajiasu_wait.gif
  File dadajiasu_wait.png
  File dadajiasu_waitBk.png
  File ddRedHotLog.png
  File ddRedLog.png
  File ddmsgHotlog.png
  File ddmsglog.png
  File default.png
  File diskIcon.png
  File edit340_36.png
  File findingLog.gif
  File flowlog.png
  File freeLog.png
  File gifRedGetLog.gif
  File headercharBk.png
  File lighttip.png
  File listtui.png
  File login_Name.png
  File loginregistLog.png
  File loginwndBk.png
  File mainMenu.xml
  File manualadd.png
  File msgboxBk.png
  File nofindgameLog.png
  File nogamebk.png
  File phoneEditLog.png
  File phoneLoginLog.png
  File progressBk.png
  File progressFore.png
  File radio1.png
  File redBtn.png
  File registLog.png
  File report70-22.png
  File schbox.png
  File scorecoin.png
  File scroll.png
  File select.png
  File select110_36.png
  File setWorldLog.png
  File setworldnotebtn.png
  File signalVal.png
  File startAccbtn.png
  File stopAccBk.png
  File subWorldBtn.png
  File tieba.png
  File trayMenuBk.png
  File traymenu.xml
  File updateProgBk.png
  File updateProgFore.png
  File updateWndBk.png
  File weixin.png
  File weixin_QRCODE.png
  File wndAccModeSel.xml
  File wndChargeTipBk.png
  File wndFloatBkBig.png
  File wndFloatBkSmall.png
  File wndHistoryLines.xml
  File wndSortType.xml
  File worldnotebk.png
  File xinlang27.png
  File xinlang27Min.png
  SetOutPath $_OUTDIR\skin\DadaPcMonitor
  File Close.png
  File GameBtnC150_36.png
  File GameBtnC150_36.png.png
  File GameBtnL150_36.png
  File GameBtnR150_36.png
  File GameBtnR150_36.png.png
  File Item.xml
  File Menu.xml
  File MonitorWnd.xml
  File NetMonitorBk.png
  File Ok110_36.png
  File Wnd.xml
  File check.png
  File flowGreen.gif
  File flowRed.gif
  File kill.png
  File open.png
  File scroll.png
  SetOutPath $_OUTDIR\skin\DadaPcMonitor\UI
  File Close.png
  File ListHeadSep.png
  File TabLabel.png
  File kill.png
  File open.png
  SetOutPath $_OUTDIR\skin\DadaPcMonitor\UI\Default
  File allWnd_bk.png
  File scroll.png
  SetOutPath $_OUTDIR\skin\DadaTray
  File WidgetMenu.xml
  File WidgetWnd.xml
  SetOutPath $_OUTDIR\skin\DadaTray\UI
  File Ball_Slide.png
  File BoardBk.jpg
  File Close.jpg
  File DelayMeter_0.jpg
  File DelayMeter_1.jpg
  File DelayMeter_2.jpg
  File DelayMeter_3.jpg
  File ToExit.jpg
  File ToHide.jpg
  File ToMain.jpg
  File ToStart.jpg
  File ToStop.jpg
  SetOutPath $_OUTDIR\skin\DadaTray\UI\Ball
  File 01.png
  File 02.png
  File 03.png
  File 04.png
  File 05.png
  File 06.png
  File 07.png
  File 08.png
  File 09.png
  File 10.png
  File 11.png
  File 12.png
  File 13.png
  File 14.png
  File 15.png
  File Idle.png
  SetOutPath $_OUTDIR\tun
  SetOutPath $_OUTDIR\tun\xp
  File OemWin2k.inf
  File devcon.exe
  File tap0901.cat
  File tap0901.sys
  SetOutPath $_OUTDIR\tun\xpabove
  SetOutPath $_OUTDIR\tun\xpabove\x64
  File OemVista.inf
  File devcon.exe
  File tap0901.cat
  File tap0901.sys
  SetOutPath $_OUTDIR\tun\xpabove\x86
  File OemVista.inf
  File devcon.exe
  File tap0901.cat
  File tap0901.sys
  SetOutPath $_OUTDIR\update
  SetOutPath $_OUTDIR
  Call func_1478
  Call func_1451
  Call func_1503
  Call func_1620
  Call func_2040
  StrCpy $0 &code1=$_19_&code2=$_20_
  StrCpy $0 http://api.dadajiasu.com/api/psn/install?ver=5.6.19.816&safe=0&mac=$_15_&downchannel=$_18_$0
  nsDui::NaviUrl $0 3
    ; Call Initialize_____Plugins
    ; SetOverwrite off
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 3
    ; Push $0
    ; CallInstDLL $PLUGINSDIR\nsDui.dll NaviUrl
  nsDui::SetVisible btn_close 1
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 1
    ; Push btn_close
    ; CallInstDLL $PLUGINSDIR\nsDui.dll SetVisible
  nsDui::SetVisible btn_min 1
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 1
    ; Push btn_min
    ; CallInstDLL $PLUGINSDIR\nsDui.dll SetVisible
  StrCmp $_17_ 1 0 label_894
  Call func_1434
  Goto label_896
label_894:
  StrCmp $_17_ 3 0 label_896
  Call func_1427
label_896:
SectionEnd


Function .onInit
  Push $EXEFILE
  Push @
  Push _
  Push -1
  Call :label_903
  Goto label_1042
label_903:
  Exch $2
    ; Push $2
    ; Exch
    ; Pop $2
  Exch
  Exch $1
    ; Push $1
    ; Exch
    ; Pop $1
  Exch
  Exch 2
  Exch $0
    ; Push $0
    ; Exch
    ; Pop $0
  Exch 2
  Exch 3
  Exch $R0
    ; Push $R0
    ; Exch
    ; Pop $R0
  Exch 3
  Push $3
  Push $4
  Push $5
  Push $6
  Push $7
  Push $8
  Push $9
  Push $R1
  Push $R2
  ClearErrors
  StrCpy $R2 ""
label_932:
  StrCpy $3 $2 1
  StrCpy $2 $2 "" 1
  StrCmp $3 E 0 label_937
  StrCpy $R2 E
  Goto label_932
label_937:
  StrCmp $3 + label_942
  StrCmp $3 - label_942
  StrCmp $3 "#" label_955
  StrCmp $3 / label_955
  Goto label_1018
label_942:
  StrCpy $4 $2 2 -2
  StrCmp $4 {{ label_952
  StrCmp $4 }} label_952
  StrCmp $4 {* label_952
  StrCmp $4 *{ label_952
  StrCmp $4 *} label_952
  StrCmp $4 }* label_952
  StrCmp $4 {} label_952
  StrCpy $4 ""
  Goto label_953
label_952:
  StrCpy $2 $2 -2
label_953:
  IntOp $2 $2 + 0
  StrCmp $2 0 label_1020
label_955:
  StrCmp $R0 "" label_1022
  StrCpy $5 -1
  StrCpy $6 0
  StrCpy $7 ""
  StrLen $8 $0
  StrLen $9 $1
label_961:
  IntOp $5 $5 + 1
label_962:
  StrCpy $R1 $R0 $8 $5
  StrCmp $R1$6 0 label_1022
  StrCmp $R1 "" label_985
  StrCmp $R1 $0 label_967
  StrCmp $7 "" label_961 label_972
label_967:
  StrCmp $0 $1 0 label_969
  StrCmp $7 "" 0 label_972
label_969:
  IntOp $7 $5 + $8
  StrCpy $5 $7
  Goto label_962
label_972:
  StrCpy $R1 $R0 $9 $5
  StrCmp $R1 $1 0 label_961
  IntOp $6 $6 + 1
  StrCmp $3$6 +$2 label_993
  StrCmp $3 / 0 label_982
  IntOp $R1 $5 - $7
  StrCpy $R1 $R0 $R1 $7
  StrCmp $R1 $2 0 label_982
  StrCpy $R1 $6
  Goto label_1025
label_982:
  IntOp $5 $5 + $9
  StrCpy $7 ""
  Goto label_962
label_985:
  StrCmp $3 - 0 label_990
  StrCpy $3 +
  IntOp $2 $6 - $2
  IntOp $2 $2 + 1
  IntCmp $2 0 label_1020 label_1020 label_955
label_990:
  StrCmp $3 "#" 0 label_1020
  StrCpy $R1 $6
  Goto label_1025
label_993:
  StrCmp $4 "" 0 label_997
  IntOp $R1 $5 - $7
  StrCpy $R1 $R0 $R1 $7
  Goto label_1025
label_997:
  IntOp $5 $5 + $9
  IntOp $7 $7 - $8
  StrCmp $4 {* label_1001
  StrCmp $4 *{ 0 label_1003
label_1001:
  StrCpy $R1 $R0 $5
  Goto label_1025
label_1003:
  StrCmp $4 *} label_1005
  StrCmp $4 }* 0 label_1007
label_1005:
  StrCpy $R1 $R0 "" $7
  Goto label_1025
label_1007:
  StrCmp $4 }} 0 label_1010
  StrCpy $R1 $R0 "" $5
  Goto label_1025
label_1010:
  StrCmp $4 {{ 0 label_1013
  StrCpy $R1 $R0 $7
  Goto label_1025
label_1013:
  StrCmp $4 {} 0 label_1018
  StrCpy $5 $R0 "" $5
  StrCpy $7 $R0 $7
  StrCpy $R1 $7$5
  Goto label_1025
label_1018:
  StrCpy $R1 3
  Goto label_1023
label_1020:
  StrCpy $R1 2
  Goto label_1023
label_1022:
  StrCpy $R1 1
label_1023:
  StrCmp $R2 E 0 label_1026
  SetErrors
label_1025:
  StrCpy $R0 $R1
label_1026:
  Pop $R2
  Pop $R1
  Pop $9
  Pop $8
  Pop $7
  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
  Exch $R0
    ; Push $R0
    ; Exch
    ; Pop $R0
  Return

label_1042:
  Pop $_18_
  StrCmp $EXEFILE $_18_ 0 label_1050
  Push $EXEFILE
  Push _
  Push _
  Push -1
  Call :label_903
  Pop $_18_
label_1050:
  StrCmp $EXEFILE $_18_ 0 label_1052
  StrCpy $_18_ ""
label_1052:
  nsDui::NewDUISetup 哒哒网游加速器安装向导 install.xml
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push install.xml
    ; Push 哒哒网游加速器安装向导
    ; CallInstDLL $PLUGINSDIR\nsDui.dll NewDUISetup
  Pop $_14_
  Call :label_1061
  Goto label_1094
label_1061:
  StrCmp $CMDLINE "" 0 label_1064
  Push ""
  Return

label_1064:
  Push $0
  Push $1
  Push $2
  Push $3
  StrLen $1 $CMDLINE
  StrCpy $2 2
  StrCpy $3 $CMDLINE 1
  StrCmp $3 $\" label_1073
  StrCpy $3 " "
label_1073:
  IntCmp $2 $1 label_1077 0 label_1077
  StrCpy $0 $CMDLINE 1 $2
  IntOp $2 $2 + 1
  StrCmp $3 $0 0 label_1073
label_1077:
  IntCmp $2 $1 label_1082 0 label_1082
  StrCpy $0 $CMDLINE 1 $2
  StrCmp $0 " " 0 label_1082
  IntOp $2 $2 + 1
  Goto label_1077
label_1082:
  StrCpy $0 $CMDLINE "" $2
label_1083:
  StrCpy $1 $0 1 -1
  StrCmp $1 " " 0 label_1087
  StrCpy $0 $0 -1
  Goto label_1083
label_1087:
  Pop $3
  Pop $2
  Pop $1
  Exch $0
    ; Push $0
    ; Exch
    ; Pop $0
  Return

label_1094:
  Pop $_17_
  ReadRegStr $R6 HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\DaDaJiaSuSetup UninstallString
  StrCmp $R6 "" label_1100
  StrLen $0 \uninst.exe
  StrCpy $INSTDIR $R6 -$0
  Call func_2035
label_1100:
  Call func_1475
FunctionEnd


Function func_1102    ; Page 0, Pre
  nsDui::FindControl btn_close
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push btn_close
    ; CallInstDLL $PLUGINSDIR\nsDui.dll FindControl
  Pop $0
  StrCmp $0 0 0 label_1116
  StrCpy $0 1428
  nsDui::BindNSIS btn_close $0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push $0
    ; Push btn_close
    ; CallInstDLL $PLUGINSDIR\nsDui.dll BindNSIS
label_1116:
  nsDui::FindControl btn_min
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push btn_min
    ; CallInstDLL $PLUGINSDIR\nsDui.dll FindControl
  Pop $0
  StrCmp $0 0 0 label_1130
  StrCpy $0 1433
  nsDui::BindNSIS btn_min $0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push $0
    ; Push btn_min
    ; CallInstDLL $PLUGINSDIR\nsDui.dll BindNSIS
label_1130:
  nsDui::FindControl btn_fast_inst
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push btn_fast_inst
    ; CallInstDLL $PLUGINSDIR\nsDui.dll FindControl
  Pop $0
  StrCmp $0 0 0 label_1144
  StrCpy $0 1369
  nsDui::BindNSIS btn_fast_inst $0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push $0
    ; Push btn_fast_inst
    ; CallInstDLL $PLUGINSDIR\nsDui.dll BindNSIS
label_1144:
  nsDui::FindControl opt_agree
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push opt_agree
    ; CallInstDLL $PLUGINSDIR\nsDui.dll FindControl
  Pop $0
  StrCmp $0 0 0 label_1158
  StrCpy $0 1323
  nsDui::BindNSIS opt_agree $0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push $0
    ; Push opt_agree
    ; CallInstDLL $PLUGINSDIR\nsDui.dll BindNSIS
label_1158:
  nsDui::FindControl btn_user_path
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push btn_user_path
    ; CallInstDLL $PLUGINSDIR\nsDui.dll FindControl
  Pop $0
  StrCmp $0 0 0 label_1172
  StrCpy $0 1356
  nsDui::BindNSIS btn_user_path $0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push $0
    ; Push btn_user_path
    ; CallInstDLL $PLUGINSDIR\nsDui.dll BindNSIS
label_1172:
  nsDui::FindControl btn_license
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push btn_license
    ; CallInstDLL $PLUGINSDIR\nsDui.dll FindControl
  Pop $0
  StrCmp $0 0 0 label_1186
  StrCpy $0 1321
  nsDui::BindNSIS btn_license $0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push $0
    ; Push btn_license
    ; CallInstDLL $PLUGINSDIR\nsDui.dll BindNSIS
label_1186:
  nsDui::FindControl btn_chg_path
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push btn_chg_path
    ; CallInstDLL $PLUGINSDIR\nsDui.dll FindControl
  Pop $0
  StrCmp $0 0 0 label_1200
  StrCpy $0 1438
  nsDui::BindNSIS btn_chg_path $0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push $0
    ; Push btn_chg_path
    ; CallInstDLL $PLUGINSDIR\nsDui.dll BindNSIS
label_1200:
  nsDui::FindControl btn_next
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push btn_next
    ; CallInstDLL $PLUGINSDIR\nsDui.dll FindControl
  Pop $0
  StrCmp $0 0 0 label_1214
  StrCpy $0 1390
  nsDui::BindNSIS btn_next $0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push $0
    ; Push btn_next
    ; CallInstDLL $PLUGINSDIR\nsDui.dll BindNSIS
label_1214:
  nsDui::FindControl btn_back
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push btn_back
    ; CallInstDLL $PLUGINSDIR\nsDui.dll FindControl
  Pop $0
  StrCmp $0 0 0 label_1228
  StrCpy $0 1413
  nsDui::BindNSIS btn_back $0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push $0
    ; Push btn_back
    ; CallInstDLL $PLUGINSDIR\nsDui.dll BindNSIS
label_1228:
  nsDui::FindControl btn_finish
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push btn_finish
    ; CallInstDLL $PLUGINSDIR\nsDui.dll FindControl
  Pop $0
  StrCmp $0 0 0 label_1242
  StrCpy $0 1435
  nsDui::BindNSIS btn_finish $0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push $0
    ; Push btn_finish
    ; CallInstDLL $PLUGINSDIR\nsDui.dll BindNSIS
label_1242:
  nsDui::SetDirValue $INSTDIR
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push $INSTDIR
    ; CallInstDLL $PLUGINSDIR\nsDui.dll SetDirValue
  nsDui::InstPage wizardTab txt_info1 1
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 1
    ; Push txt_info1
    ; Push wizardTab
    ; CallInstDLL $PLUGINSDIR\nsDui.dll InstPage
  StrCmp $_17_ 1 0 label_1269
  Call func_2008
  nsDui::InstPage wizardTab txt_info1 0
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 0
    ; Push txt_info1
    ; Push wizardTab
    ; CallInstDLL $PLUGINSDIR\nsDui.dll InstPage
  nsDui::NextPage 2
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 2
    ; CallInstDLL $PLUGINSDIR\nsDui.dll NextPage
  Goto label_1290
label_1269:
  StrCmp $_17_ 3 0 label_1289
  nsDui::ShowWnd 0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 0
    ; CallInstDLL $PLUGINSDIR\nsDui.dll ShowWnd
  Call func_1958
  nsDui::InstPage wizardTab txt_info1 0
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 0
    ; Push txt_info1
    ; Push wizardTab
    ; CallInstDLL $PLUGINSDIR\nsDui.dll InstPage
  nsDui::NextPage 2
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 2
    ; CallInstDLL $PLUGINSDIR\nsDui.dll NextPage
  Goto label_1290
label_1289:
  Call func_2008
label_1290:
  nsDui::ShowPage
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; CallInstDLL $PLUGINSDIR\nsDui.dll ShowPage
FunctionEnd


Function func_1295    ; Page 1, Show
  ShowWindow $HWNDPARENT ${SW_HIDE}
  System::Call user32::SetWindowPos(i$HWNDPARENT,i0,i0,i0,i0,i0,i0x0002)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push user32::SetWindowPos(i$HWNDPARENT,i0,i0,i0,i0,i0,i0x0002)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  nsDui::InstBindNSIS Slider_Percent txt_percent
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push txt_percent
    ; Push Slider_Percent
    ; CallInstDLL $PLUGINSDIR\nsDui.dll InstBindNSIS
  nsDui::SetVisible btn_close 0
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 0
    ; Push btn_close
    ; CallInstDLL $PLUGINSDIR\nsDui.dll SetVisible
  nsDui::SetVisible btn_min 0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 0
    ; Push btn_min
    ; CallInstDLL $PLUGINSDIR\nsDui.dll SetVisible
  Return

  ExecShell open http://www.dadajiasu.com/regxy    ; "open http://www.dadajiasu.com/regxy"
  Return

  nsDui::GetChecked opt_agree
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push opt_agree
    ; CallInstDLL $PLUGINSDIR\nsDui.dll GetChecked
  Pop $0
  StrCmp $0 0 0 label_1342
  nsDui::SetEnabled btn_fast_inst 1
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 1
    ; Push btn_fast_inst
    ; CallInstDLL $PLUGINSDIR\nsDui.dll SetEnabled
  nsDui::SetEnabled btn_user_path 1
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 1
    ; Push btn_user_path
    ; CallInstDLL $PLUGINSDIR\nsDui.dll SetEnabled
  Goto label_1354
label_1342:
  nsDui::SetEnabled btn_fast_inst 0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 0
    ; Push btn_fast_inst
    ; CallInstDLL $PLUGINSDIR\nsDui.dll SetEnabled
  nsDui::SetEnabled btn_user_path 0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 0
    ; Push btn_user_path
    ; CallInstDLL $PLUGINSDIR\nsDui.dll SetEnabled
label_1354:
  Return

  nsDui::GetChecked opt_agree
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push opt_agree
    ; CallInstDLL $PLUGINSDIR\nsDui.dll GetChecked
  Pop $0
  StrCmp $0 1 0 label_1367
  nsDui::NextPage 1
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 1
    ; CallInstDLL $PLUGINSDIR\nsDui.dll NextPage
label_1367:
  Return

  nsDui::GetDirValue
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; CallInstDLL $PLUGINSDIR\nsDui.dll GetDirValue
  Pop $0
  StrCmp $0 "" label_1388
  StrCpy $INSTDIR $0
  Call func_2035
  nsDui::InstPage wizardTab txt_info1 0
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 0
    ; Push txt_info1
    ; Push wizardTab
    ; CallInstDLL $PLUGINSDIR\nsDui.dll InstPage
  nsDui::NextPage 2
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 2
    ; CallInstDLL $PLUGINSDIR\nsDui.dll NextPage
label_1388:
  Return

  nsDui::GetDirValue
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; CallInstDLL $PLUGINSDIR\nsDui.dll GetDirValue
  Pop $0
  StrCmp $0 "" label_1410
  StrCpy $INSTDIR $0
  Call func_2035
  nsDui::InstPage wizardTab txt_info1 0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 0
    ; Push txt_info1
    ; Push wizardTab
    ; CallInstDLL $PLUGINSDIR\nsDui.dll InstPage
  nsDui::NextPage 1
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 1
    ; CallInstDLL $PLUGINSDIR\nsDui.dll NextPage
  Goto label_1411
label_1410:
  MessageBox MB_OK|MB_ICONSTOP 请选择正确的路径.
label_1411:
  Return

  nsDui::GetDirValue
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; CallInstDLL $PLUGINSDIR\nsDui.dll GetDirValue
  Pop $0
  StrCmp $0 "" label_1425
  StrCpy $INSTDIR $0
  Call func_2035
  nsDui::PrePage
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; CallInstDLL $PLUGINSDIR\nsDui.dll PrePage
  Goto label_1426
label_1425:
  MessageBox MB_OK|MB_ICONSTOP 请选择正确的路径.
label_1426:
FunctionEnd


Function func_1427
  nsDui::ExitDUISetup
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; CallInstDLL $PLUGINSDIR\nsDui.dll ExitDUISetup
  Return

  SendMessage $_14_ 0x0112 0xF020 0
FunctionEnd


Function func_1434
  ExecShell open $INSTDIR\DaDaJiaSu.exe    ; "open $INSTDIR\DaDaJiaSu.exe"
  Call func_1427
  Return

  nsDui::SelectInstallDir
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; CallInstDLL $PLUGINSDIR\nsDui.dll SelectInstallDir
  Pop $0
  StrCmp $0 "" label_1450
  StrCpy $INSTDIR $0
  Call func_2035
  nsDui::SetDirValue $INSTDIR
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push $INSTDIR
    ; CallInstDLL $PLUGINSDIR\nsDui.dll SetDirValue
label_1450:
FunctionEnd


Function func_1451
  nsisFirewall::AddAuthorizedApplication $INSTDIR\DaDaJiaSu.exe DaDaJiaSu
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\nsisFirewall.dll
    ; SetDetailsPrint lastused
    ; Push DaDaJiaSu
    ; Push $INSTDIR\DaDaJiaSu.exe
    ; CallInstDLL $PLUGINSDIR\nsisFirewall.dll AddAuthorizedApplication
  SetShellVarContext all
  CreateDirectory $SMPROGRAMS\哒哒网游加速器
  CreateShortCut $SMPROGRAMS\哒哒网游加速器\哒哒网游加速器.lnk $INSTDIR\DaDaJiaSu.exe
  CreateShortCut $DESKTOP\哒哒网游加速器.lnk $INSTDIR\DaDaJiaSu.exe
  CreateShortCut $SMPROGRAMS\哒哒网游加速器\卸载哒哒网游加速器.lnk $INSTDIR\uninst.exe
  WriteRegStr HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\DaDaJiaSuSetup DisplayName DaDaJiaSuSetup
  WriteRegStr HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\DaDaJiaSuSetup UninstallString $INSTDIR\uninst.exe
  WriteRegStr HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\DaDaJiaSuSetup DisplayIcon $INSTDIR\DaDaJiaSu.exe
  WriteRegStr HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\DaDaJiaSuSetup DisplayVersion 5.6.19.816
  WriteRegStr HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\DaDaJiaSuSetup Channel $_18_
  System::Call "ole32::CoCreateGuid(g .s)"
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "ole32::CoCreateGuid(g .s)"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $0
  WriteRegStr HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\DaDaJiaSuSetup UUID $0
FunctionEnd


Function func_1475
  CopyFiles /SILENT $INSTDIR\cfgs\*.* $TEMP\dd_file_translate\cfgs    ; $(LSTR_7)$TEMP\dd_file_translate\cfgs    ;  "复制到: "
  CopyFiles /SILENT $INSTDIR\CacheBuf\*.* $TEMP\dd_file_translate\CacheBuf    ; $(LSTR_7)$TEMP\dd_file_translate\CacheBuf    ;  "复制到: "
FunctionEnd


Function func_1478
  ReadINIStr $1 $TEMP\dd_file_translate\cfgs\cfgs.ini autologin account
  ReadINIStr $2 $TEMP\dd_file_translate\cfgs\cfgs.ini autologin pswd
  ReadINIStr $3 $TEMP\dd_file_translate\cfgs\cfgs.ini login auto
  ReadINIStr $4 $TEMP\dd_file_translate\cfgs\cfgs.ini login remember
  ReadINIStr $5 $TEMP\dd_file_translate\cfgs\cfgs.ini global PwdAcc
  ReadINIStr $6 $TEMP\dd_file_translate\cfgs\cfgs.ini global LasterLoginByPhone
  ReadINIStr $7 $TEMP\dd_file_translate\cfgs\cfgs.ini global PwdPhone
  ReadINIStr $8 $TEMP\dd_file_translate\cfgs\cfgs.ini global LoginPhone
  ReadINIStr $9 $TEMP\dd_file_translate\cfgs\cfgs.ini autologin UserID
  WriteINIStr $INSTDIR\cfgs\cfgs.ini autologin account $1
  WriteINIStr $INSTDIR\cfgs\cfgs.ini autologin pswd $2
  WriteINIStr $INSTDIR\cfgs\cfgs.ini login auto $3
  WriteINIStr $INSTDIR\cfgs\cfgs.ini login remember $4
  WriteINIStr $INSTDIR\cfgs\cfgs.ini global PwdAcc $5
  WriteINIStr $INSTDIR\cfgs\cfgs.ini global LasterLoginByPhone $6
  WriteINIStr $INSTDIR\cfgs\cfgs.ini global PwdPhone $7
  WriteINIStr $INSTDIR\cfgs\cfgs.ini global LoginPhone $8
  WriteINIStr $INSTDIR\cfgs\cfgs.ini autologin UserID $9
  CopyFiles /SILENT $TEMP\dd_file_translate\cfgs\games.xml $INSTDIR\cfgs    ; $(LSTR_7)$INSTDIR\cfgs    ;  "复制到: "
  CopyFiles /SILENT $TEMP\dd_file_translate\cfgs\mygame.ini $INSTDIR\cfgs    ; $(LSTR_7)$INSTDIR\cfgs    ;  "复制到: "
  CopyFiles /SILENT $TEMP\dd_file_translate\cfgs\mygame.xml $INSTDIR\cfgs    ; $(LSTR_7)$INSTDIR\cfgs    ;  "复制到: "
  CopyFiles /SILENT $TEMP\dd_file_translate\cfgs\MyGameConfigure.xml $INSTDIR\cfgs    ; $(LSTR_7)$INSTDIR\cfgs    ;  "复制到: "
  CopyFiles /SILENT $TEMP\dd_file_translate\CacheBuf\*.* $INSTDIR\CacheBuf    ; $(LSTR_7)$INSTDIR\CacheBuf    ;  "复制到: "
  RMDir /r $TEMP\dd_file_translate
FunctionEnd


Function func_1503
  System::Call Iphlpapi::GetAdaptersInfo(i,*i.r0)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push Iphlpapi::GetAdaptersInfo(i,*i.r0)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Alloc $0
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push $0
    ; CallInstDLL $PLUGINSDIR\System.dll Alloc
  Pop $1
  System::Call Iphlpapi::GetAdaptersInfo(ir1r2,*ir0)i.r0
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push Iphlpapi::GetAdaptersInfo(ir1r2,*ir0)i.r0
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  StrCmp $0 0 0 label_1546
label_1520:
  StrCmp $2 0 label_1546
  System::Call *$2(i.r2,i,&t260.s,&t132.s,i.r5)i.r0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push *$2(i.r2,i,&t260.s,&t132.s,i.r5)i.r0
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  IntOp $3 403 + $5
  StrCpy $6 ""
  StrCpy $4 404
  Goto label_1531
label_1530:
  IntOp $4 $4 + 1
label_1531:
  IntCmp $4 $3 0 0 label_1544
  IntOp $7 $0 + $4
  System::Call *$7(&i1.r7)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push *$7(&i1.r7)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  IntFmt $7 %02X $7
  StrCpy $6 $6$7
  StrCmp $4 $3 label_1542
  StrCpy $6 $6
label_1542:
  Goto label_1530
  Goto label_1530
label_1544:
  StrCpy $_15_ $6
  Goto label_1520
label_1546:
  System::Free $1
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push $1
    ; CallInstDLL $PLUGINSDIR\System.dll Free
FunctionEnd


Function func_1552
  System::Alloc 16
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push 16
    ; CallInstDLL $PLUGINSDIR\System.dll Alloc
  System::Call kernel32::GetLocalTime(isR0)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetLocalTime(isR0)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call *$R0(&i2.R1,&i2.R2,&i2,&i2.R4,&i2.R5,&i2.R6,&i2.R7,&i2.R8)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push *$R0(&i2.R1,&i2.R2,&i2,&i2.R4,&i2.R5,&i2.R6,&i2.R7,&i2.R8)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Free $R0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push $R0
    ; CallInstDLL $PLUGINSDIR\System.dll Free
  StrCpy $_16_ $R1$R2$R4$R5$R6$R7$R8
  ExecCmd::exec "taskkill /IM DaDaJiaSu.exe /F"
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\ExecCmd.dll
    ; SetDetailsPrint lastused
    ; Push "taskkill /IM DaDaJiaSu.exe /F"
    ; CallInstDLL $PLUGINSDIR\ExecCmd.dll exec
  ExecCmd::exec "taskkill /IM LSPHelper64.bin /F"
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\ExecCmd.dll
    ; SetDetailsPrint lastused
    ; Push "taskkill /IM LSPHelper64.bin /F"
    ; CallInstDLL $PLUGINSDIR\ExecCmd.dll exec
  ExecCmd::exec "taskkill /IM LSPHelper.bin /F"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\ExecCmd.dll
    ; SetDetailsPrint lastused
    ; Push "taskkill /IM LSPHelper.bin /F"
    ; CallInstDLL $PLUGINSDIR\ExecCmd.dll exec
  ExecCmd::exec "taskkill /IM CrashReport.exe /F"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\ExecCmd.dll
    ; SetDetailsPrint lastused
    ; Push "taskkill /IM CrashReport.exe /F"
    ; CallInstDLL $PLUGINSDIR\ExecCmd.dll exec
  ExecCmd::exec "taskkill /IM DaDaDiagnosis.exe /F"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\ExecCmd.dll
    ; SetDetailsPrint lastused
    ; Push "taskkill /IM DaDaDiagnosis.exe /F"
    ; CallInstDLL $PLUGINSDIR\ExecCmd.dll exec
  ExecCmd::exec "taskkill /IM EchoClient.exe /F"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\ExecCmd.dll
    ; SetDetailsPrint lastused
    ; Push "taskkill /IM EchoClient.exe /F"
    ; CallInstDLL $PLUGINSDIR\ExecCmd.dll exec
  IfFileExists $INSTDIR\*.exe 0 label_1605
  Rename $INSTDIR\*.exe $INSTDIR\*.exe$_16_.old ;  $INSTDIR\*.exe->$INSTDIR\*.exe$_16_.old
label_1605:
  IfFileExists $INSTDIR\*.dll 0 label_1607
  Rename $INSTDIR\*.dll $INSTDIR\*.dll$_16_.old ;  $INSTDIR\*.dll->$INSTDIR\*.dll$_16_.old
label_1607:
  IfFileExists $INSTDIR\*.bin 0 label_1609
  Rename $INSTDIR\*.bin $INSTDIR\*.bin$_16_.old ;  $INSTDIR\*.bin->$INSTDIR\*.bin$_16_.old
label_1609:
  IfFileExists $INSTDIR\*.exe*.old 0 label_1611
  Delete /REBOOTOK $INSTDIR\*.exe*.old
label_1611:
  IfFileExists $INSTDIR\*.dll*.old 0 label_1613
  Delete /REBOOTOK $INSTDIR\*.dll*.old
label_1613:
  IfFileExists $INSTDIR\*.bin*.old 0 label_1615
  Delete /REBOOTOK $INSTDIR\*.bin*.old
label_1615:
  IfFileExists $INSTDIR\60\*.sys 0 label_1617
  Delete /REBOOTOK $INSTDIR\60\*.sys
label_1617:
  IfFileExists $INSTDIR\50\*.sys 0 label_1619
  Delete /REBOOTOK $INSTDIR\50\*.sys
label_1619:
FunctionEnd


Function func_1620
  System::Call kernel32::GetCurrentProcess()i.s
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetCurrentProcess()i.s
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call kernel32::IsWow64Process(is,*i.s)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::IsWow64Process(is,*i.s)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $_21_
  StrCmp $_21_ 0 label_1637
  System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 0)"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "Kernel32::Wow64EnableWow64FsRedirection(i 0)"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
label_1637:
  Push ""
  Push L
  Call :label_1641
  Goto label_1810
label_1641:
  Exch $1
    ; Push $1
    ; Exch
    ; Pop $1
  Exch
  Exch $0
    ; Push $0
    ; Exch
    ; Pop $0
  Exch
  Push $2
  Push $3
  Push $4
  Push $5
  Push $6
  Push $7
  ClearErrors
  StrCmp $1 L label_1681
  StrCmp $1 A label_1665
  StrCmp $1 C label_1665
  StrCmp $1 M label_1665
  StrCmp $1 LS label_1681
  StrCmp $1 AS label_1665
  StrCmp $1 CS label_1665
  StrCmp $1 MS label_1665
  Goto label_1773
label_1665:
  IfFileExists $0 0 label_1773
  System::Call "*(i,l,l,l,i,i,i,i,&t260,&t14) i .r6"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "*(i,l,l,l,i,i,i,i,&t260,&t14) i .r6"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call "kernel32::FindFirstFileA(t,i)i(r0,r6) .r2"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "kernel32::FindFirstFileA(t,i)i(r0,r6) .r2"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call kernel32::FindClose(i)i(r2)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::FindClose(i)i(r2)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
label_1681:
  System::Call "*(&i2,&i2,&i2,&i2,&i2,&i2,&i2,&i2) i .r7"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "*(&i2,&i2,&i2,&i2,&i2,&i2,&i2,&i2) i .r7"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  StrCmp $1 L 0 label_1693
  System::Call kernel32::GetLocalTime(i)i(r7)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetLocalTime(i)i(r7)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Goto label_1734
label_1693:
  StrCmp $1 LS 0 label_1700
  System::Call kernel32::GetSystemTime(i)i(r7)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetSystemTime(i)i(r7)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Goto label_1734
label_1700:
  System::Call *$6(i,l,l,l,i,i,i,i,&t260,&t14)i(,.r4,.r3,.r2)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push *$6(i,l,l,l,i,i,i,i,&t260,&t14)i(,.r4,.r3,.r2)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Free $6
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push $6
    ; CallInstDLL $PLUGINSDIR\System.dll Free
  StrCmp $1 A 0 label_1713
  StrCpy $2 $3
  Goto label_1724
label_1713:
  StrCmp $1 C 0 label_1716
  StrCpy $2 $4
  Goto label_1724
label_1716:
  StrCmp $1 M label_1724
  StrCmp $1 AS label_1729
  StrCmp $1 CS 0 label_1721
  StrCpy $3 $4
  Goto label_1729
label_1721:
  StrCmp $1 MS 0 label_1724
  StrCpy $3 $2
  Goto label_1729
label_1724:
  System::Call kernel32::FileTimeToLocalFileTime(*l,*l)i(r2,.r3)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::FileTimeToLocalFileTime(*l,*l)i(r2,.r3)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
label_1729:
  System::Call kernel32::FileTimeToSystemTime(*l,i)i(r3,r7)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::FileTimeToSystemTime(*l,i)i(r3,r7)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
label_1734:
  System::Call *$7(&i2,&i2,&i2,&i2,&i2,&i2,&i2,&i2)i(.r5,.r6,.r4,.r0,.r3,.r2,.r1,)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push *$7(&i2,&i2,&i2,&i2,&i2,&i2,&i2,&i2)i(.r5,.r6,.r4,.r0,.r3,.r2,.r1,)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Free $7
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push $7
    ; CallInstDLL $PLUGINSDIR\System.dll Free
  IntCmp $0 9 0 0 label_1746
  StrCpy $0 0$0
label_1746:
  IntCmp $1 9 0 0 label_1748
  StrCpy $1 0$1
label_1748:
  IntCmp $2 9 0 0 label_1750
  StrCpy $2 0$2
label_1750:
  IntCmp $6 9 0 0 label_1752
  StrCpy $6 0$6
label_1752:
  StrCmp $4 0 0 label_1755
  StrCpy $4 Sunday
  Goto label_1781
label_1755:
  StrCmp $4 1 0 label_1758
  StrCpy $4 Monday
  Goto label_1781
label_1758:
  StrCmp $4 2 0 label_1761
  StrCpy $4 Tuesday
  Goto label_1781
label_1761:
  StrCmp $4 3 0 label_1764
  StrCpy $4 Wednesday
  Goto label_1781
label_1764:
  StrCmp $4 4 0 label_1767
  StrCpy $4 Thursday
  Goto label_1781
label_1767:
  StrCmp $4 5 0 label_1770
  StrCpy $4 Friday
  Goto label_1781
label_1770:
  StrCmp $4 6 0 label_1773
  StrCpy $4 Saturday
  Goto label_1781
label_1773:
  SetErrors
  StrCpy $0 ""
  StrCpy $1 ""
  StrCpy $2 ""
  StrCpy $3 ""
  StrCpy $4 ""
  StrCpy $5 ""
  StrCpy $6 ""
label_1781:
  Pop $7
  Exch $6
    ; Push $6
    ; Exch
    ; Pop $6
  Exch
  Exch $5
    ; Push $5
    ; Exch
    ; Pop $5
  Exch 2
  Exch $4
    ; Push $4
    ; Exch
    ; Pop $4
  Exch 3
  Exch $3
    ; Push $3
    ; Exch
    ; Pop $3
  Exch 4
  Exch $2
    ; Push $2
    ; Exch
    ; Pop $2
  Exch 5
  Exch $1
    ; Push $1
    ; Exch
    ; Pop $1
  Exch 6
  Exch $0
    ; Push $0
    ; Exch
    ; Pop $0
  Return

label_1810:
  Pop $0
  Pop $1
  Pop $2
  Pop $3
  Pop $4
  Pop $5
  Pop $6
  StrCpy $7 $2$1$0$4$5$6
  System::Call kernel32::GetCurrentProcess()i.s
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetCurrentProcess()i.s
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call kernel32::IsWow64Process(is,*i.s)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::IsWow64Process(is,*i.s)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $_21_
  StrCmp $_21_ 0 label_1839
  IfFileExists $SYSDIR\DDIP64.dll 0 label_1832
  Rename $SYSDIR\DDIP64.dll $SYSDIR\DDIP64.dll$7.old ;  $SYSDIR\DDIP64.dll->$SYSDIR\DDIP64.dll$7.old
label_1832:
  IfFileExists $WINDIR\SysWOW64\DDIP.dll 0 label_1834
  Rename $WINDIR\SysWOW64\DDIP.dll $WINDIR\SysWOW64\DDIP.dll$7.old ;  $WINDIR\SysWOW64\DDIP.dll->$WINDIR\SysWOW64\DDIP.dll$7.old
label_1834:
  IfFileExists $SYSDIR\DDIP64.dll*.old 0 label_1836
  Delete /REBOOTOK $SYSDIR\DDIP64.dll*.old
label_1836:
  IfFileExists $WINDIR\SysWOW64\DDIP.dll*.old 0 label_1838
  Delete /REBOOTOK $WINDIR\SysWOW64\DDIP.dll*.old
label_1838:
  Goto label_1843
label_1839:
  IfFileExists $SYSDIR\DDIP.dll 0 label_1841
  Rename $SYSDIR\DDIP.dll $SYSDIR\DDIP.dll$7.old ;  $SYSDIR\DDIP.dll->$SYSDIR\DDIP.dll$7.old
label_1841:
  IfFileExists $SYSDIR\DDIP.dll*.old 0 label_1843
  Delete /REBOOTOK $SYSDIR\DDIP.dll*.old
label_1843:
  System::Call kernel32::GetCurrentProcess()i.s
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetCurrentProcess()i.s
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call kernel32::IsWow64Process(is,*i.s)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::IsWow64Process(is,*i.s)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $_21_
  StrCmp $_21_ 0 label_1858
  CopyFiles /SILENT $INSTDIR\DDIP64.dll $SYSDIR    ; $(LSTR_7)$SYSDIR    ;  "复制到: "
  CopyFiles /SILENT $INSTDIR\DDIP.dll $WINDIR\SysWOW64    ; $(LSTR_7)$WINDIR\SysWOW64    ;  "复制到: "
  Goto label_1859
label_1858:
  CopyFiles /SILENT $INSTDIR\DDIP.dll $SYSDIR    ; $(LSTR_7)$SYSDIR    ;  "复制到: "
label_1859:
  System::Call kernel32::GetCurrentProcess()i.s
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetCurrentProcess()i.s
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call kernel32::IsWow64Process(is,*i.s)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::IsWow64Process(is,*i.s)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $_21_
  StrCmp $_21_ 0 label_1876
  System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 1)"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "Kernel32::Wow64EnableWow64FsRedirection(i 1)"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
label_1876:
FunctionEnd


Function func_1877
  System::Alloc 16
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push 16
    ; CallInstDLL $PLUGINSDIR\System.dll Alloc
  System::Call kernel32::GetLocalTime(isR0)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetLocalTime(isR0)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call *$R0(&i2.R1,&i2.R2,&i2,&i2.R4,&i2.R5,&i2.R6,&i2.R7,&i2.R8)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push *$R0(&i2.R1,&i2.R2,&i2,&i2.R4,&i2.R5,&i2.R6,&i2.R7,&i2.R8)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Free $R0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push $R0
    ; CallInstDLL $PLUGINSDIR\System.dll Free
  StrCpy $_16_ $R1$R2$R4$R5$R6$R7$R8
  System::Call kernel32::GetCurrentProcess()i.s
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetCurrentProcess()i.s
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call kernel32::IsWow64Process(is,*i.s)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::IsWow64Process(is,*i.s)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $_21_
  StrCmp $_21_ 0 label_1915
  System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 0)"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "Kernel32::Wow64EnableWow64FsRedirection(i 0)"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
label_1915:
  System::Call kernel32::GetCurrentProcess()i.s
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetCurrentProcess()i.s
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call kernel32::IsWow64Process(is,*i.s)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::IsWow64Process(is,*i.s)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $_21_
  StrCmp $_21_ 0 label_1936
  IfFileExists $SYSDIR\DDIP64.dll 0 label_1929
  Rename $SYSDIR\DDIP64.dll $SYSDIR\DDIP64.dll$_16_.old ;  $SYSDIR\DDIP64.dll->$SYSDIR\DDIP64.dll$_16_.old
label_1929:
  IfFileExists $WINDIR\SysWOW64\DDIP.dll 0 label_1931
  Rename $WINDIR\SysWOW64\DDIP.dll $WINDIR\SysWOW64\DDIP.dll$_16_.old ;  $WINDIR\SysWOW64\DDIP.dll->$WINDIR\SysWOW64\DDIP.dll$_16_.old
label_1931:
  IfFileExists $SYSDIR\DDIP64.dll*.old 0 label_1933
  Delete /REBOOTOK $SYSDIR\DDIP64.dll*.old
label_1933:
  IfFileExists $WINDIR\SysWOW64\DDIP.dll*.old 0 label_1935
  Delete /REBOOTOK $WINDIR\SysWOW64\DDIP.dll*.old
label_1935:
  Goto label_1940
label_1936:
  IfFileExists $SYSDIR\DDIP.dll 0 label_1938
  Rename $SYSDIR\DDIP.dll $SYSDIR\DDIP.dll$_16_.old ;  $SYSDIR\DDIP.dll->$SYSDIR\DDIP.dll$_16_.old
label_1938:
  IfFileExists $SYSDIR\DDIP.dll*.old 0 label_1940
  Delete /REBOOTOK $SYSDIR\DDIP.dll*.old
label_1940:
  System::Call kernel32::GetCurrentProcess()i.s
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetCurrentProcess()i.s
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call kernel32::IsWow64Process(is,*i.s)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::IsWow64Process(is,*i.s)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $_21_
  StrCmp $_21_ 0 label_1957
  System::Call "Kernel32::Wow64EnableWow64FsRedirection(i 1)"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push "Kernel32::Wow64EnableWow64FsRedirection(i 1)"
    ; CallInstDLL $PLUGINSDIR\System.dll Call
label_1957:
FunctionEnd


Function func_1958
  Call func_1552
  Call func_1973
  SetOutPath $TEMP
  SetShellVarContext all
  Call func_1877
  RMDir /r $INSTDIR
  RMDir /r $SMPROGRAMS\哒哒网游加速器
  Delete $DESKTOP\哒哒网游加速器.lnk
  DeleteRegKey HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\DaDaJiaSuSetup
  nsisFirewall::RemoveAuthorizedApplication $INSTDIR\DaDaJiaSu.exe
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\nsisFirewall.dll
    ; SetDetailsPrint lastused
    ; Push $INSTDIR\DaDaJiaSu.exe
    ; CallInstDLL $PLUGINSDIR\nsisFirewall.dll RemoveAuthorizedApplication
FunctionEnd


Function func_1973
  System::Call kernel32::GetCurrentProcess()i.s
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetCurrentProcess()i.s
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call kernel32::IsWow64Process(is,*i.s)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::IsWow64Process(is,*i.s)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $_21_
  StrCmp $_21_ 0 label_2001
  System::Call kernel32::Wow64EnableWow64FsRedirection(i0)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::Wow64EnableWow64FsRedirection(i0)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  nsExec::ExecToLog /timeout=5000 "netsh winsock reset"
    ; Call Initialize_____Plugins
    ; AllowSkipFiles on
    ; File $PLUGINSDIR\nsExec.dll
    ; SetDetailsPrint lastused
    ; Push "netsh winsock reset"
    ; Push /timeout=5000
    ; CallInstDLL $PLUGINSDIR\nsExec.dll ExecToLog
  System::Call kernel32::Wow64EnableWow64FsRedirection(i1)
    ; Call Initialize_____Plugins
    ; AllowSkipFiles off
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::Wow64EnableWow64FsRedirection(i1)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
label_2001:
  nsExec::ExecToLog /timeout=5000 "netsh winsock reset"
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsExec.dll
    ; SetDetailsPrint lastused
    ; Push "netsh winsock reset"
    ; Push /timeout=5000
    ; CallInstDLL $PLUGINSDIR\nsExec.dll ExecToLog
FunctionEnd


Function func_2008
  ReadRegStr $R6 HKLM Software\Microsoft\Windows\CurrentVersion\Uninstall\DaDaJiaSuSetup UninstallString
  StrCmp $R6 "" label_2029 label_2010
label_2010:
  nsDui::ShowWnd 0
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 0
    ; CallInstDLL $PLUGINSDIR\nsDui.dll ShowWnd
  StrCmp $_17_ "" 0 label_2017
  MessageBox MB_YESNO|MB_ICONEXCLAMATION $\r$\n安装文件已存在，请先卸载原先版本。是否现在进行卸载？$\r$\n IDYES label_2017 IDNO label_2028
label_2017:
  StrLen $0 \uninst.exe
  StrCpy $INSTDIR $R6 -$0
  ExecWait "$R6 1"
  Call func_1958
  nsDui::SetDirValue $INSTDIR
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push $INSTDIR
    ; CallInstDLL $PLUGINSDIR\nsDui.dll SetDirValue
  ExecShell open $EXEPATH    ; "open $EXEPATH"
  Goto label_2029
label_2028:
  Call func_1427
label_2029:
  nsDui::ShowWnd 5
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\nsDui.dll
    ; SetDetailsPrint lastused
    ; Push 5
    ; CallInstDLL $PLUGINSDIR\nsDui.dll ShowWnd
FunctionEnd


Function func_2035
  StrLen $0 DaDaJiaSu
  StrCpy $5 $INSTDIR "" -$0
  StrCmp $5 DaDaJiaSu label_2039
  StrCpy $INSTDIR $INSTDIR\DaDaJiaSu
label_2039:
FunctionEnd


Function func_2040
  ReadRegStr $_19_ HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion" ProductName
  System::Call kernel32::GetCurrentProcess()i.s
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::GetCurrentProcess()i.s
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  System::Call kernel32::IsWow64Process(is,*i.s)
    ; Call Initialize_____Plugins
    ; File $PLUGINSDIR\System.dll
    ; SetDetailsPrint lastused
    ; Push kernel32::IsWow64Process(is,*i.s)
    ; CallInstDLL $PLUGINSDIR\System.dll Call
  Pop $_21_
  StrCmp $_21_ 0 label_2055
  StrCpy $_20_ x64
  Goto label_2056
label_2055:
  StrCpy $_20_ x86
label_2056:
FunctionEnd


/*
Function Initialize_____Plugins
  SetDetailsPrint none
  StrCmp $PLUGINSDIR "" 0 label_2067
  Push $0
  SetErrors
  GetTempFileName $0
  Delete $0
  CreateDirectory $0
  IfErrors label_2068
  StrCpy $PLUGINSDIR $0
  Pop $0
label_2067:
  Return

label_2068:
  MessageBox MB_OK|MB_ICONSTOP "Error! Can't initialize plug-ins directory. Please try again later." /SD IDOK
  Quit
FunctionEnd
*/



; --------------------
; UNREFERENCED STRINGS:

/*
34 $PROGRAMFILES
38 CommonFilesDir
53 "$PROGRAMFILES\Common Files"
70 $COMMONFILES
*/
