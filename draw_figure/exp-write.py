# coding=utf-8

import numpy as np
import matplotlib.pyplot as plt
import xlrd
from matplotlib import gridspec
import copy

plt.rcParams['font.sans-serif'] = ['Arial']  # 如果要显示中文字体,则在此处设为：SimHei
plt.rcParams['axes.unicode_minus'] = False  # 显示负号
plt.rcParams['xtick.bottom'] = False
colors = ["#ffea67","#ffb548","red","#a6092e","#ffea67","#ffb548","red","#a6092e"]
excel = xlrd.open_workbook("exp-write.xlsx")
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
        temp.append(j/128)
    xs.append(tempx)
    ys.append(temp)
for ind, x in enumerate(xs):
    for i in range(len(x)):
        xs[ind][i] += ind * bar_width
ys_normal = copy.deepcopy(ys)

for ind1, y in enumerate(ys):
    for ind2, i in enumerate(y):
        ys_normal[ind1][ind2] /= ys[0][ind2]
        ys_normal[ind1][ind2] = str(round(ys_normal[ind1][ind2]))+"x"
# print(ys_normal)
print(labels)
print(xs)
print(ys)
labelx = list(sheet.row_values(0)[1:])

tickx = np.array(xs[1]) + 0.5 * bar_width
print(tickx)
print(labelx)
# 建图
fig = plt.figure(figsize=(6.5, 2.2))
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
    bars.append(
        plt.bar(xs[i], ys[i], width=bar_width, label=labels[i], hatch="", ec="black",lw=1, color=colors[i], log=False))
    # if (i % 3 == 2):
    #     for j in range(len(xs[i])):
    #         if (j == 5):
    #             plt.text(xs[i][j] - 0.8 * bar_width, ys[i][j] + 10, ys_normal[i][j], fontsize=12)
    #         else:
    #             plt.text(xs[i][j] - 0.8 * bar_width, ys[i][j] + 10, ys_normal[i][j], fontsize=12)
# 画x刻度
plt.xticks(tickx, labelx, rotation=0, fontsize=14)
plt.yticks([0,2,4,6,8,10],fontsize=12)
# plt.xlim(-0.2,7)
# plt.ylim(0,1280)
plt.ylabel('MQPS', fontsize=14, fontweight='bold')
plt.xlabel('Sequential write                               Random Write   ',fontsize=14, fontweight='bold')
# plt.legend((bars[0], bars[3], bars[1], bars[4], bars[2], bars[5]),
#            (labels[0], labels[3], labels[1], labels[4], labels[2], labels[5]), ncol=3, fontsize=14, borderpad=False,
#            framealpha=1, labelspacing=0.1, columnspacing=0.5,
#            handletextpad=0.5,loc=4,bbox_to_anchor=(1, 0.8))
plt.legend(ncol=1, fontsize=14, borderpad=False,framealpha=1, labelspacing=0.1, columnspacing=0.5,handletextpad=0.1)
leg = plt.gca().get_legend()
ltext = leg.get_texts()
plt.setp(ltext, fontsize=14, fontweight='bold')  # 设置图例字体的大小和粗细
# ax_sub = gca.twinx()
# ax_sub.set_ylabel('MQPS',fontsize=14,fontweight='bold')
# ax_sub.set_ylim(0,10)
# plt.yticks(fontsize=14)
fig.tight_layout()
plt.savefig('./exp-write.pdf', format='pdf')
plt.show()
