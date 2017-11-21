#ifndef FGFL_LEX_H
#define FGFL_LEX_H

#include <stdint.h>

enum {
	TNONE,
	TG_IDENT,
	TL_IDENT,
	TEQUAL,
	TCOM,
	TSPACE,
	TSEMI,
	TIGCASE,
	TCOMMA,
	TEOF,
};

#define TOTAL_TOKEN	10

#define START_STATE	1
#define DEAD_STATE	0

//Size of state table = 17
static uint8_t state_table[][256] = {
	{},
	{[9]=2, [10]=2, [32]=2, [35]=3, [37]=4, [44]=5, [59]=6, [61]=7, [64]=8, [65]=9, [66]=9, [67]=9, [68]=9, [69]=9, [70]=9, [71]=9, [72]=9, [73]=9, [74]=9, [75]=9, [76]=9, [77]=9, [78]=9, [79]=9, [80]=9, [81]=9, [82]=9, [83]=9, [84]=9, [85]=9, [86]=9, [87]=9, [88]=9, [89]=9, [90]=9, [95]=9, [97]=9, [98]=9, [99]=9, [100]=9, [101]=9, [102]=9, [103]=9, [104]=9, [105]=9, [106]=9, [107]=9, [108]=9, [109]=9, [110]=9, [111]=9, [112]=9, [113]=9, [114]=9, [115]=9, [116]=9, [117]=9, [118]=9, [119]=9, [120]=9, [121]=9, [122]=9, },
	{[9]=2, [10]=2, [32]=2, },
	{[0]=3, [1]=3, [2]=3, [3]=3, [4]=3, [5]=3, [6]=3, [7]=3, [8]=3, [9]=3, [11]=3, [12]=3, [13]=3, [14]=3, [15]=3, [16]=3, [17]=3, [18]=3, [19]=3, [20]=3, [21]=3, [22]=3, [23]=3, [24]=3, [25]=3, [26]=3, [27]=3, [28]=3, [29]=3, [30]=3, [31]=3, [32]=3, [33]=3, [34]=3, [35]=3, [36]=3, [37]=3, [38]=3, [39]=3, [40]=3, [41]=3, [42]=3, [43]=3, [44]=3, [45]=3, [46]=3, [47]=3, [48]=3, [49]=3, [50]=3, [51]=3, [52]=3, [53]=3, [54]=3, [55]=3, [56]=3, [57]=3, [58]=3, [59]=3, [60]=3, [61]=3, [62]=3, [63]=3, [64]=3, [65]=3, [66]=3, [67]=3, [68]=3, [69]=3, [70]=3, [71]=3, [72]=3, [73]=3, [74]=3, [75]=3, [76]=3, [77]=3, [78]=3, [79]=3, [80]=3, [81]=3, [82]=3, [83]=3, [84]=3, [85]=3, [86]=3, [87]=3, [88]=3, [89]=3, [90]=3, [91]=3, [92]=3, [93]=3, [94]=3, [95]=3, [96]=3, [97]=3, [98]=3, [99]=3, [100]=3, [101]=3, [102]=3, [103]=3, [104]=3, [105]=3, [106]=3, [107]=3, [108]=3, [109]=3, [110]=3, [111]=3, [112]=3, [113]=3, [114]=3, [115]=3, [116]=3, [117]=3, [118]=3, [119]=3, [120]=3, [121]=3, [122]=3, [123]=3, [124]=3, [125]=3, [126]=3, [127]=3, [128]=3, [129]=3, [130]=3, [131]=3, [132]=3, [133]=3, [134]=3, [135]=3, [136]=3, [137]=3, [138]=3, [139]=3, [140]=3, [141]=3, [142]=3, [143]=3, [144]=3, [145]=3, [146]=3, [147]=3, [148]=3, [149]=3, [150]=3, [151]=3, [152]=3, [153]=3, [154]=3, [155]=3, [156]=3, [157]=3, [158]=3, [159]=3, [160]=3, [161]=3, [162]=3, [163]=3, [164]=3, [165]=3, [166]=3, [167]=3, [168]=3, [169]=3, [170]=3, [171]=3, [172]=3, [173]=3, [174]=3, [175]=3, [176]=3, [177]=3, [178]=3, [179]=3, [180]=3, [181]=3, [182]=3, [183]=3, [184]=3, [185]=3, [186]=3, [187]=3, [188]=3, [189]=3, [190]=3, [191]=3, [192]=3, [193]=3, [194]=3, [195]=3, [196]=3, [197]=3, [198]=3, [199]=3, [200]=3, [201]=3, [202]=3, [203]=3, [204]=3, [205]=3, [206]=3, [207]=3, [208]=3, [209]=3, [210]=3, [211]=3, [212]=3, [213]=3, [214]=3, [215]=3, [216]=3, [217]=3, [218]=3, [219]=3, [220]=3, [221]=3, [222]=3, [223]=3, [224]=3, [225]=3, [226]=3, [227]=3, [228]=3, [229]=3, [230]=3, [231]=3, [232]=3, [233]=3, [234]=3, [235]=3, [236]=3, [237]=3, [238]=3, [239]=3, [240]=3, [241]=3, [242]=3, [243]=3, [244]=3, [245]=3, [246]=3, [247]=3, [248]=3, [249]=3, [250]=3, [251]=3, [252]=3, [253]=3, [254]=3, [255]=3, },
	{[105]=10, },
	{},
	{},
	{},
	{[65]=11, [66]=11, [67]=11, [68]=11, [69]=11, [70]=11, [71]=11, [72]=11, [73]=11, [74]=11, [75]=11, [76]=11, [77]=11, [78]=11, [79]=11, [80]=11, [81]=11, [82]=11, [83]=11, [84]=11, [85]=11, [86]=11, [87]=11, [88]=11, [89]=11, [90]=11, [95]=11, [97]=11, [98]=11, [99]=11, [100]=11, [101]=11, [102]=11, [103]=11, [104]=11, [105]=11, [106]=11, [107]=11, [108]=11, [109]=11, [110]=11, [111]=11, [112]=11, [113]=11, [114]=11, [115]=11, [116]=11, [117]=11, [118]=11, [119]=11, [120]=11, [121]=11, [122]=11, },
	{[48]=9, [49]=9, [50]=9, [51]=9, [52]=9, [53]=9, [54]=9, [55]=9, [56]=9, [57]=9, [65]=9, [66]=9, [67]=9, [68]=9, [69]=9, [70]=9, [71]=9, [72]=9, [73]=9, [74]=9, [75]=9, [76]=9, [77]=9, [78]=9, [79]=9, [80]=9, [81]=9, [82]=9, [83]=9, [84]=9, [85]=9, [86]=9, [87]=9, [88]=9, [89]=9, [90]=9, [95]=9, [97]=9, [98]=9, [99]=9, [100]=9, [101]=9, [102]=9, [103]=9, [104]=9, [105]=9, [106]=9, [107]=9, [108]=9, [109]=9, [110]=9, [111]=9, [112]=9, [113]=9, [114]=9, [115]=9, [116]=9, [117]=9, [118]=9, [119]=9, [120]=9, [121]=9, [122]=9, },
	{[103]=12, },
	{[48]=11, [49]=11, [50]=11, [51]=11, [52]=11, [53]=11, [54]=11, [55]=11, [56]=11, [57]=11, [65]=11, [66]=11, [67]=11, [68]=11, [69]=11, [70]=11, [71]=11, [72]=11, [73]=11, [74]=11, [75]=11, [76]=11, [77]=11, [78]=11, [79]=11, [80]=11, [81]=11, [82]=11, [83]=11, [84]=11, [85]=11, [86]=11, [87]=11, [88]=11, [89]=11, [90]=11, [95]=11, [97]=11, [98]=11, [99]=11, [100]=11, [101]=11, [102]=11, [103]=11, [104]=11, [105]=11, [106]=11, [107]=11, [108]=11, [109]=11, [110]=11, [111]=11, [112]=11, [113]=11, [114]=11, [115]=11, [116]=11, [117]=11, [118]=11, [119]=11, [120]=11, [121]=11, [122]=11, },
	{[99]=13, },
	{[97]=14, },
	{[115]=15, },
	{[101]=16, },
	{},
};

static uint8_t final_table[][2] = {
	{ 2, 	TSPACE },
	{ 3, 	TCOM },
	{ 5, 	TCOMMA },
	{ 6, 	TSEMI },
	{ 7, 	TEQUAL },
	{ 9, 	TG_IDENT },
	{ 11, 	TL_IDENT },
	{ 16, 	TIGCASE },
};

#define SIZE_FINAL_TAB	8

#endif /* FGFL_LEX_H */