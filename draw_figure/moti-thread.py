# coding=utf-8

import numpy as np
import matplotlib.pyplot as plt
import xlrd
from matplotlib import gridspec

plt.rcParams['font.sans-serif'] = ['Arial']  # 如果要显示中文字体,则在此处设为：SimHei
plt.rcParams['axes.unicode_minus'] = False  # 显示负号
plt.rcParams['xtick.bottom'] = False
colors = ["g", "orange"]
# colors = ["#140032","#358582", "#7ACABB", "#FFFFAA"]
excel = xlrd.open_workbook("moti-threads.xlsx")
sheet = excel.sheet_by_index(0)
labels = []
ys = []
xs = []
index = []
bar_width = 0.3
# 数据
for i in range(1, sheet.nrows):
    row = sheet.row_values(i)
    temp = []
    tempx = []
    labels.append(row[0])
    for ind, j in enumerate(row[1:]):
        tempx.append(ind)
        temp.append(j)
    xs.append(tempx)
    ys.append(temp)
for ind, x in enumerate(xs):
    for i in range(len(x)):
        xs[ind][i] += ind * bar_width
print(labels)
print(xs)
print(ys)
labelx = [int(x) for x in list(sheet.row_values(0)[1:])]

tickx = np.array(xs[0]) + 0.5 * bar_width
print(tickx)
print(labelx)
# 建图
fig = plt.figure(figsize=(6, 2.2))
# gs = gridspec.GridSpec(1, 3, width_ratios=[3, 1,1])
# ax1=plt.subplot(gs[0])
gca = plt.gca()
gca.set_axisbelow(True)
gca.grid(axis='y', linestyle='dotted', lw=1, alpha=1)
gca.spines['top'].set_visible(False)  # 去掉上边框
gca.spines['right'].set_visible(False)  # 去掉右边框

# 画柱子
for i in range(len(xs)):
    # for x in range(len(xs[i])):
    #     xs[i][x] += i * bar_width
    plt.bar(xs[i], ys[i], width=bar_width, label=labels[i], ec="black", color=colors[i], log=False)
# plt.text(xs[0][-1]-bar_width*1.5,2050,"3707")
# 画x刻度
plt.xticks(tickx, labelx, rotation=0, fontsize=14)
plt.yticks(fontsize=14)
# plt.xlim(-0.25,2.75)
plt.ylim(0,1500)
plt.ylabel('KQPS', fontsize=14, fontweight='bold')
plt.xlabel('Number of threads',fontsize=14, fontweight='bold')
plt.legend(fontsize=14, borderpad=0.3, framealpha=1)
leg = plt.gca().get_legend()
ltext = leg.get_texts()
plt.setp(ltext, fontsize=14, fontweight='bold')  # 设置图例字体的大小和粗细

# plt.legend(loc=1,fontsize=8,ncol=4,bbox_to_anchor=(0.81, 1),borderpad=False,framealpha=0)
# plt.ylim(0,2000)
fig.tight_layout()
plt.savefig('./moti-thread.pdf', format='pdf')
plt.show()
