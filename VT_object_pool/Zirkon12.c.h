// ISO-Designer ISO 11783   Version 5.7.2.7013 Bucher Automation AG
// Do not change!

#define WorkingSet_0_Offset                                     1
#define Aktivierung_Startmaske_Offset                          21
#define Aktivierung_Einstellungsmaske_Offset                   34
#define Macro_3_Offset                                         47
#define Macro_4_Offset                                         68
#define sel_gear2_Offset                                       89
#define sel_gear1_Offset                                      102
#define On_Arbeitsscheinwerfer_Offset                         115
#define Off_Arbeitsscheinwerfer_Offset                        128
#define reset_speed_back_to_zero_Offset                       141
#define Startmaske_Offset                                     154
#define Einstellungen_Offset                                  216
#define Container_3000_Offset                                 248
#define Container_3001_Offset                                 276
#define Container_3002_Offset                                 298
#define Container_3003_Offset                                 326
#define Container_3004_Offset                                 354
#define Container_3005_Offset                                 388
#define SM_Navigation_Offset                                  422
#define SoftKey_Startseite_Offset                             432
#define SoftKey_Einstellungen_Offset                          447
#define btn_gear_change_Offset                                462
#define btn_light_Offset                                      485
#define InputBoolean_use_gnss_speed_Offset                    508
#define OS_Ueberschrift_Offset                                525
#define OutputString_11001_Offset                             551
#define OutputString_11002_Offset                             585
#define OutputString_11003_Offset                             618
#define OutputString_11004_Offset                             652
#define OutputString_speed_Offset                             684
#define OutputString_einstellungen_Offset                     716
#define OutputString_kmh_Offset                               746
#define OutputString_11008_Offset                             767
#define ON_pto_rpm_Offset                                     809
#define ON_tine_rpm_Offset                                    838
#define ON_btn_gear_Offset                                    867
#define ON_sel_gear_Offset                                    896
#define OutputNumber_RPM_per_m_Offset                         925
#define OutputNumber_gnss_speed_kmh_Offset                    954
#define OutputNumber_PTO_RPM_dial_Offset                      983
#define OutputNumber_speed_Offset                            1012
#define Rectangle_14000_Offset                               1041
#define Rectangle_14001_Offset                               1054
#define Rectangle_14003_Offset                               1067
#define ArchedBargraph_19000_Offset                          1080
#define PowerHarrow_small_20000_Offset                       1107
#define IMG_Home_Icon_Offset                                 3788
#define IMG_Einstellungen_Icon_Offset                        4307
#define Arbeitsscheinwerfer_grey_Background_20003_Offset     4972
#define Arbeitsscheinwerfer_green_Background_20004_Offset    6051
#define NV_PTO_RPM_Offset                                    6998
#define NV_Tine_RPM_Offset                                   7005
#define NV_btn_gear_Offset                                   7012
#define NV_speed_Offset                                      7019
#define NV_gnss_speed_Offset                                 7026
#define NV_sel_gear_Offset                                   7033
#define NV_tine_rpm_per_m_21006_Offset                       7040
#define NV_use_gnss_bool_Offset                              7047
#define FA_Ueberschrifft_Offset                              7054
#define FA_RPM_Offset                                        7062
#define FA_BEschriftung_23002_Offset                         7070
#define FontAttributes_23003_Offset                          7078
#define LA_Ueberschrift_Offset                               7086
#define LA_background_Offset                                 7094
#define LA_border_output_Offset                              7102
#define FillAttributes_25000_Offset                          7110
#define FA_border_output_Offset                              7118
#define OP_light_Offset                                      7126

#define ISO_OP_MEMORY_CLASS

#define ISO_OP_Zirkon12_Size   7131
extern const unsigned char ISO_OP_MEMORY_CLASS isoOP_Zirkon12[];

#define ISO_OP_Zirkon12_ObjectNumber     68
extern const unsigned long ISO_OP_MEMORY_CLASS isoOP_Zirkon12_Offset[];
extern const unsigned long ISO_OP_MEMORY_CLASS isoOP_Zirkon12_Offset_Id[];
#define ISO_OP_Zirkon12_Scale_Offset      1

#define ID_NULL  0xFFFF

#define TYPEID_WORKSET        0
#define TYPEID_DATAMASK       1
#define TYPEID_ALARMMASK      2
#define TYPEID_CONTAINER      3
#define TYPEID_SKEYMASK       4
#define TYPEID_SOFTKEY        5
#define TYPEID_BUTTON         6
#define TYPEID_INBOOL         7
#define TYPEID_INSTR          8
#define TYPEID_INNUM          9
#define TYPEID_INLIST        10
#define TYPEID_OUTSTR        11
#define TYPEID_OUTNUM        12
#define TYPEID_OUTLINE       13
#define TYPEID_OUTRECT       14
#define TYPEID_OUTELLIPSE    15
#define TYPEID_OUTPOLY       16
#define TYPEID_OUTMETER      17
#define TYPEID_OUTLINBAR     18
#define TYPEID_OUTARCBAR     19
#define TYPEID_OUTPICT       20
#define TYPEID_VARNUM        21
#define TYPEID_VARSTR        22
#define TYPEID_FONTATTR      23
#define TYPEID_LINEATTR      24
#define TYPEID_FILLATTR      25
#define TYPEID_INPATTR       26
#define TYPEID_OBJPTR        27
#define TYPEID_MACRO         28
#define TYPEID_AUXFUNC       29
#define TYPEID_AUXINP        30
#define TYPEID_AUXFUNC2      31
#define TYPEID_AUXINP2       32
#define TYPEID_AUXPTR        33
#define TYPEID_WINMASK       34
#define TYPEID_KEYGROUP      35
#define TYPEID_GRPHCTXT      36
#define TYPEID_OUTLIST       37
#define TYPEID_EXTINPATTR    38
#define TYPEID_COLORMAP      39
#define TYPEID_OBJLBLREF     40
#define TYPEID_EXTOBJDEF     41
#define TYPEID_EXTREFNAME    42
#define TYPEID_EXTOBJPTR     43
#define TYPEID_ANIMATION     44
#define TYPEID_COLORPAL      45
#define TYPEID_GRAPHDATA     46
#define TYPEID_WSSPECIAL     47
#define TYPEID_SCALEGRAPH    48

#define EV_REFRESH            0
#define EV_ACT                1
#define EV_DEACT              2
#define EV_SHOW               3
#define EV_HIDE               4
#define EV_ENABLE             5
#define EV_DISABLE            6
#define EV_CHGACTMASK         7
#define EV_CHGSKEYMASK        8
#define EV_CHGATTR            9
#define EV_CHGBKCOLOR        10
#define EV_CHGFONTATTR       11
#define EV_CHGLINEATTR       12
#define EV_CHGFILLATTR       13
#define EV_CHGCHILDLOC       14
#define EV_CHGSIZE           15
#define EV_CHGVAL            16
#define EV_CHGPRIOR          17
#define EV_CHGENDPNT         18
#define EV_SELINPUT          19
#define EV_DESELINPUT        20
#define EV_ESC               21
#define EV_ENTERVAL          22
#define EV_ENTERCHGVAL       23
#define EV_KEYPRESS          24
#define EV_KEYRELEASE        25
#define EV_CHGCHILDPOS       26

#define CMD_HIDE_SHOW               160
#define CMD_ENABLE_DISABLE          161
#define CMD_SELECT_INPUT_OBJECT     162
#define CMD_CONTROL_AUDIO_DEVICE    163
#define CMD_SET_AUDIO_VOLUME        164
#define CMD_CHANGE_CHILD_LOCATION   165
#define CMD_CHANGE_SIZE             166
#define CMD_CHANGE_BACKGROUND_COLOR 167
#define CMD_CHANGE_NUMERIC_VALUE    168
#define CMD_CHANGE_END_POINT        169
#define CMD_CHANGE_FONT_ATTRIBUTES  170
#define CMD_CHANGE_LINE_ATTRIBUTES  171
#define CMD_CHANGE_FILL_ATTRIBUTES  172
#define CMD_CHANGE_ACTIVE_MASK      173
#define CMD_CHANGE_SOFT_KEY_MASK    174
#define CMD_CHANGE_ATTRIBUTE        175
#define CMD_CHANGE_PRIORITY         176
#define CMD_CHANGE_LIST_ITEM        177
#define CMD_CHANGE_STRING_VALUE     179
#define CMD_CHANGE_CHILD_POSITION   180
#define CMD_SET_OBJECT_LABEL        181
#define CMD_CHANGE_POLYGON_POINT    182
#define CMD_CHANGE_POLYGON_SCALE    183
#define CMD_GRAPHICS_CONTEXT        184
#define CMD_GET_ATTRIBUTE           185
#define CMD_SELECT_COLOURMAP_OR_PALETTE 186
#define CMD_EXECUTE_EXTENDED_MACRO  188
#define CMD_LOCK_UNLOCK_MASK        189
#define CMD_EXECUTE_MACRO           190

#define COLOR_BLACK       0
#define COLOR_WHITE       1
#define COLOR_GREEN       2
#define COLOR_TEAL        3
#define COLOR_MAROON      4
#define COLOR_PURPLE      5
#define COLOR_OLIVE       6
#define COLOR_SILVER      7
#define COLOR_GREY        8
#define COLOR_BLUE        9
#define COLOR_LIME       10
#define COLOR_CYAN       11
#define COLOR_RED        12
#define COLOR_MAGENTA    13
#define COLOR_YELLOW     14
#define COLOR_NAVY       15

#define FLOAT_1      0x00, 0x00, 0x80, 0x3F
#define FLOAT_10     0x00, 0x00, 0x20, 0x41
#define FLOAT_100    0x00, 0x00, 0xC8, 0x42
#define FLOAT_1000   0x00, 0x00, 0x7A, 0x44
#define FLOAT_0_1    0xCD, 0xCC, 0xCC, 0x3D
#define FLOAT_0_01   0x0A, 0xD7, 0x23, 0x3C
#define FLOAT_0_001  0x6F, 0x12, 0x83, 0x3A
