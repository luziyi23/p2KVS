# coding=utf-8

import numpy as np
import matplotlib.pyplot as plt
import pylab as pl

plt.rcParams['font.sans-serif'] = ['Arial']  # 如果要显示中文字体,则在此处设为：SimHei
plt.rcParams['axes.unicode_minus'] = False  # 显示负号
plt.rcParams['xtick.bottom'] = False

# F1k=[100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 78.79, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100]
# B128=[52.53, 36.5, 0, 7.5, 0, 7.5, 21.225, 45.11, 29.535, 0, 8.415, 2.02, 5, 28.065, 44.12, 54.175, 39, 8.08, 6.635, 0, 40.545, 45.5, 53.56, 46.12, 0, 7.92, 0, 51.94, 52.265, 43.94, 53.675, 9.9, 6.635, 11.09, 49.1, 53.67, 44.5, 52.58, 27.275, 7.92, 19.48, 44.9, 53.955, 44.39, 53.21, 21, 8, 21, 43.295, 53.795, 43.5, 52.08, 38.755, 8, 29.28, 44.66, 53.305, 44, 51.32, 6.565, 0.99]
F1k=[100, 100, 99, 100, 100, 100, 100, 100, 100, 99, 100, 100, 100, 81, 6, 5.05, 100, 100, 100, 100, 36.63, 4.08, 3.09, 43.56, 100, 100, 98.99, 100, 52.53, 5.05, 4.04, 7, 9.09, 100, 100, 100, 99, 100, 45, 3.06, 5.88, 4, 0, 50.51, 67.35, 69.31, 69.31, 68.63, 44, 4.95, 1.02, 0, 0, 6, 44.9, 99.01, 100, 100, 100, 100, 15.84]
B1k=[42.015, 35.07, 48.53, 48.285, 48.555, 46.44, 48.04, 45.255, 49.86, 45.9, 48.795, 44.915, 47.4, 46.715, 38.615, 38.725, 47.46, 46.755, 46.105, 44.215, 44.12, 39.625, 38.5, 48.775, 44.045, 42.955, 46.175, 42.2, 46.595, 38.71, 39, 37.895, 40.185, 45.5, 42.59, 45.49, 45.335, 43.645, 36.695, 39.425, 38.5, 40.595, 42.03, 65.695, 63.725, 60.575, 62.25, 62.135, 50.52, 38, 41.425, 43.45, 42.2, 43.32, 47.935, 44.52, 46.54, 43.63, 45.275, 46.78, 38.415]

colors = ["#DEDEDE", "#F3EBE3", "#E9BF6A", "#14B383", "#096A9D", "#482A0C"]

index = range(0, len(F1k))

colors = ["#DEDEDE", "#F3EBE3", "#E9BF6A", "#14B383", "#096A9D", "#482A0C"]

bar_width = 0.30
fig, ax2 = plt.subplots(1, 1, figsize=(6, 2.5))
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


line1 = ax2.plot(index, list(np.array(F1k)/1), label='Foreground Thread', ls='-', linewidth = 2)
line2 = ax2.plot(index, list(np.array(B1k)/1), label='Background Thread', ls='--', linewidth = 2)

plt.ylim(0, 105)
plt.xlim(0, 100)
# ax2.set_yscale("log")
# xlabels = ['A1','A2','B','C1','C2']
xlabels = range(0, len(F1k) , 10)
xindex = range(0, len(F1k), 10)
plt.xticks(xindex, xlabels, fontsize=13)
plt.yticks(fontsize=13)
plt.minorticks_off()


# plt.text(index[0], 1.33, SIB[0], size = 10)
# plt.text(index[1], 1.33, SIB[1], size = 10)
# plt.text(index[2], 1.33, SIB[2], size = 10)
# plt.text(index[3], 1.33, SIB[3], size = 10)
# plt.text(index[4], 1.33, SIB[4], size = 10)



plt.legend(loc=1,ncol=2,fontsize=12,bbox_to_anchor=(1, 1.2),framealpha=0)
leg = ax2.get_legend()
ltext = leg.get_texts()
plt.setp(ltext, fontsize=14, fontweight='bold')  # 设置图例字体的大小和粗细
ax2.set_ylabel('CPU Utilization(%)', fontsize=15, fontweight='bold')
ax2.set_xlabel("Time(s)", fontsize=15, fontweight='bold', )
ax2.tick_params(which='both', direction='in')  # 设置刻度
ax2.tick_params(which='major', length=4)
plt.tight_layout()
plt.savefig('./bg-cpu-1KB.pdf', format='pdf')
plt.show()
