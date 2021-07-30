# coding=utf-8

import numpy as np
import matplotlib.pyplot as plt
import xlrd
from matplotlib import gridspec

plt.rcParams['font.sans-serif'] = ['Arial']  # 如果要显示中文字体,则在此处设为：SimHei
plt.rcParams['axes.unicode_minus'] = False  # 显示负号
plt.rcParams['xtick.bottom'] = False
# colors = ["g", "orange"]
colors = ["#140032","#358582", "#7ACABB", "#FFFFAA"]


# 建图
fig = plt.figure(figsize=(4, 2.5))
# gs = gridspec.GridSpec(1, 3, width_ratios=[3, 1,1])
# ax1=plt.subplot(gs[0])
gca = plt.gca()
gca.set_axisbelow(True)
gca.grid(axis='y', linestyle='dotted', lw=1, alpha=1)
gca.spines['top'].set_visible(False)  # 去掉上边框
gca.spines['right'].set_visible(False)  # 去掉右边框
#
# 画柱子
plt.bar(0, 98, width=0.8, ec="black",color="white")
plt.bar(1, 90, width=0.8, ec="black",color="white")
plt.bar(2, 79, width=0.8, ec="black",color="white")
plt.bar(3, 58, width=0.8, ec="black",color="white")
plt.bar(4, 54, width=0.8, ec="black",color="white")
plt.bar(5, 50, width=0.8, ec="black",color="white")
# plt.text(xs[0][-1]-bar_width*1.5,2050,"3707")
# 画x刻度
plt.xticks([0,1,2,3,4,5], ["128B","256B","512B","1KB","2KB","4KB","8KB","16KB"], rotation=30, fontsize=12)
plt.yticks(fontsize=12)
plt.xlim(-1,6)
# plt.ylim(0,1500)
plt.ylabel('CPU usage of the\n foreground thread(%)', fontsize=12, fontweight='bold')
plt.xlabel('Batched Request Size',fontsize=12, fontweight='bold')
# plt.legend(fontsize=14, borderpad=False, framealpha=1)
# leg = plt.gca().get_legend()
# ltext = leg.get_texts()
# plt.setp(ltext, fontsize=14, fontweight='bold')  # 设置图例字体的大小和粗细

# plt.legend(loc=1,fontsize=8,ncol=4,bbox_to_anchor=(0.81, 1),borderpad=False,framealpha=0)
# plt.ylim(0,2000)
fig.tight_layout()
plt.savefig('./moti-cpu.pdf', format='pdf')
plt.show()
