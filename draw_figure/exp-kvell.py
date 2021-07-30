# coding=utf-8

import numpy as np
import matplotlib.pyplot as plt
import pylab as pl

plt.rcParams['font.sans-serif'] = ['Arial']  # 如果要显示中文字体,则在此处设为：SimHei
plt.rcParams['axes.unicode_minus'] = False  # 显示负号
plt.rcParams['xtick.bottom'] = False

#IO
# F128=[296,337,335,329,327,320,320,314,314,309,314,311,311,307,303,304,300,301,300,303,298,296,294,293,297,293,290,294,296,296,289,289,288,283,287,283,284,279,208]
# B128=[2289,2355,1222,1011,2314,2353,2331,2325,2338,2326,2334,2317,2324,2316,2309,2311,2310,2310,2314,2326,2309,2304,2302,2309,2307,2303,2315,2341]
#Memory
# F128=[5.616,5.616,5.616,5.616,5.616,5.616,5.616,5.616,5.828,5.828,5.828,5.828,6.328,6.328,6.328,6.328,6.328,6.828,6.828,6.828,6.828,7.328,7.328,7.328,7.328,7.328,7.828,7.828,7.828,7.828,7.828,8.328,8.328,8.328,8.328,8.328,8.328,8.828,8.828,8.828]
# B128=[0.564,0.554,0.753,0.794,0.952,0.823,0.79,0.985,0.825,0.869,0.817,0.946,0.962,0.901,0.964,0.931,1.002,0.952,1.033,1.022,1.057,1.072,1.083,1.114,1.197,1.057,1.134,1.09]
#CPU
# F128=[438.9,500,438.9,477.8,483.3,488.2,488.2,455.6,676,644,611,672,753,729,741,724,788,700,735,706,747,700,753,672,735,747,741,712,735,728,678,753,741,678,683,765,694,678,689,338.9]
# B128=[1228,694.1,729.4,1239,1137,1139,1183,1178,1294,1122,1250,1194,1256,1200,1233,1317,1278,1106,1089,1156,1100,1133,1156,1211,1235,1312,1176,566.7]
#AVGCPU
F128=[54.8625,62.5,54.8625,59.725,60.4125,61.025,61.025,56.95,84.5,80.5,76.375,84,94.125,91.125,92.625,90.5,98.5,87.5,91.875,88.25,93.375,87.5,94.125,84,91.875,93.375,92.625,89,91.875,91,84.75,94.125,92.625,84.75,85.375,95.625,86.75,84.75,86.125,42.3625]
B128=[51.16666667,28.92083333,30.39166667,51.625,47.375,47.45833333,49.29166667,49.08333333,53.91666667,46.75,52.08333333,49.75,52.33333333,50,51.375,54.875,53.25,46.08333333,45.375,48.16666667,45.83333333,47.20833333,48.16666667,50.45833333,51.45833333,54.66666667,49,23.6125]

colors = ["#DEDEDE", "#F3EBE3", "#E9BF6A", "#14B383", "#096A9D", "#482A0C"]


bar_width = 0.30
fig, ax2 = plt.subplots(1, 1, figsize=(4, 2))
# plt.grid(linestyle=":")  # 设置背景网格线为虚线
gca = plt.gca()
gca.set_axisbelow(True)
gca.grid(axis='y', linestyle='dotted', lw=1, alpha=1)
gca.spines['top'].set_visible(False)  # 去掉上边框
gca.spines['right'].set_visible(False)  # 去掉右边框



# ax2.spines['top'].set_visible(False)  # 去掉上边框
# ax2.spines['right'].set_visible(False)  # 去掉右边框
# 设置图位置
plt.subplots_adjust(left=0.12)
plt.subplots_adjust(bottom=0.16)
plt.subplots_adjust(right=0.96)
plt.subplots_adjust(top=0.83)


line1 = ax2.plot(range(0,len(F128)), list(np.array(F128)/1), label='KVell-8', ls='--', linewidth = 2,color="black")
line2 = ax2.plot(range(0,len(B128)), list(np.array(B128)/1), label='p$^2$KVS-8', ls='-', linewidth = 2,color="red")
# line3 = ax2.plot(index, list(np.array(F1k)/1), label='PM_W', ls='--', marker='x')
# line4 = ax2.plot(index, list(np.array(B1k)/1), label='PM_R', ls='--', marker='+')

plt.ylim(0, )
# ax2.set_yscale("log")
# xlabels = ['A1','A2','B','C1','C2']
# plt.ylim(0,1500)
xlabels = range(0, 100 , 5)
xindex = range(0, 100, 5)
plt.xticks(xindex, xlabels, fontsize=13)
plt.yticks(fontsize=13)
plt.xlim(0, 25)

plt.minorticks_off()


# plt.text(index[0], 1.33, SIB[0], size = 10)
# plt.text(index[1], 1.33, SIB[1], size = 10)
# plt.text(index[2], 1.33, SIB[2], size = 10)
# plt.text(index[3], 1.33, SIB[3], size = 10)
# plt.text(index[4], 1.33, SIB[4], size = 10)


# plt.legend(ncol=1, bbox_to_anchor=(1.00, 1.02), edgecolor='white', fontsize=8)
plt.legend(loc=4,ncol=2,fontsize=12,bbox_to_anchor=(1, 0.85),framealpha=0)
leg = ax2.get_legend()
ltext = leg.get_texts()
plt.setp(ltext, fontsize=14, fontweight='bold')  # 设置图例字体的大小和粗细
ax2.set_ylabel('CPU per-core(%)', fontsize=12, fontweight='bold')
ax2.set_xlabel("Time(s)", fontsize=14, fontweight='bold')
ax2.tick_params(which='both', direction='in')  # 设置刻度
ax2.tick_params(which='major', length=4)
plt.tight_layout()
plt.savefig('./EXP-KVELL.pdf', format='pdf')
plt.show()
