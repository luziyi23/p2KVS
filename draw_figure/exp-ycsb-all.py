# coding=utf-8

import numpy as np
import matplotlib.pyplot as plt
import xlrd
from matplotlib import gridspec

plt.rcParams['font.sans-serif'] = ['Arial']  # 如果要显示中文字体,则在此处设为：SimHei
plt.rcParams['axes.unicode_minus'] = False  # 显示负号
plt.rcParams['xtick.bottom'] = False
colors = ["g", "deepskyblue", "red", "g", "deepskyblue", "red"]
excel = xlrd.open_workbook("exp-ycsb-all.xlsx")
sheet = excel.sheet_by_index(0)
labels = []
ys = []
xs = []
index = []
bar_width = 0.15
# labels=sheet.col_values(0)[1:]
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
        if ind > 2:
            xs[ind][i] += 0.3*bar_width
print(labels)
print(xs)
print(ys)
labelx = list(sheet.row_values(0)[1:])

tickx = np.array(xs[2]) + 0.5 * bar_width
print(tickx)
print(labelx)
# 建图
fig = plt.figure(figsize=(8, 3))
# gs = gridspec.GridSpec(1, 3, width_ratios=[3, 1,1])
# ax1=plt.subplot(gs[0])
gca = plt.gca()
gca.set_axisbelow(True)
gca.grid(axis='y', linestyle='dotted', lw=1, alpha=1)
gca.spines['top'].set_visible(False)  # 去掉上边框
gca.spines['right'].set_visible(False)  # 去掉右边框

# 画柱子
bars = []
for i in range(len(xs)):
    # for x in range(len(xs[i])):
    #     xs[i][x] += i * bar_width
    if i > 2:
        h = "//\\\\"
    else:
        h = ""
    bars.append(
        plt.bar(xs[i], ys[i], width=bar_width,edgecolor="black",lw=1, label=labels[i], hatch=h, color=colors[i], log=False))
# plt.text(xs[0][-1]-bar_width*1.5,2050,"3707")
# 画x刻度
plt.xticks(tickx, labelx, rotation=0, fontsize=14)
plt.yticks(fontsize=14)
plt.xlim(-0.2,7)
# plt.ylim(0,1500)
plt.ylabel('KQPS', fontsize=14, fontweight='bold')
# plt.xlabel('Thread number',fontsize=14)
(bars[0],bars[3],bars[1],bars[4],bars[2],bars[5]),(labels[0],labels[3],labels[1],labels[4],labels[2],labels[5])
# plt.legend(loc=4, fontsize=12, ncol=6,
#            bbox_to_anchor=(1, 0.9), borderpad=False, framealpha=0, labelspacing=0.1, columnspacing=0.5,
#            handletextpad=0.5)
plt.legend((bars[0], bars[3], bars[1], bars[4], bars[2], bars[5]),(labels[0], labels[3], labels[1], labels[4], labels[2], labels[5]),ncol=3,fontsize=14, borderpad=False,framealpha=1,labelspacing=0.5, columnspacing=0.5,
           handletextpad=0.5)
# plt.legend(fontsize=14, borderpad=False, framealpha=0)
leg = plt.gca().get_legend()
ltext = leg.get_texts()
plt.setp(ltext, fontsize=12, fontweight='bold')  # 设置图例字体的大小和粗细

fig.tight_layout()
plt.savefig('./exp-ycsb-all.pdf', format='pdf')
plt.show()
