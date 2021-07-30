# coding=utf-8

import numpy as np
import matplotlib.pyplot as plt
import xlrd
from matplotlib import gridspec
import copy

plt.rcParams['font.sans-serif'] = ['Arial']  # 如果要显示中文字体,则在此处设为：SimHei
plt.rcParams['axes.unicode_minus'] = False  # 显示负号
plt.rcParams['xtick.bottom'] = False
colors = ["white","red","white","red","#ffb548","red"]
excel = xlrd.open_workbook("exp-kvell-ycsb.xlsx")
sheet = excel.sheet_by_index(0)
labels = []
ys = []
xs = []
index = []
bar_width = 0.2
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
ys_normal = copy.deepcopy(ys)

for ind1, y in enumerate(ys):
    for ind2, i in enumerate(y):
        ys_normal[ind1][ind2] /= ys[0][ind2]
# print(ys_normal)
print(labels)
print(xs)
print(ys)
labelx = list(sheet.row_values(0)[1:])

tickx = np.array(xs[1]) + 0.5 * bar_width
print(tickx)
print(labelx)
# 建图
fig = plt.figure(figsize=(8, 2.3))
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
    if i > 1:
        h = "//\\\\"
    else:
        h = ""
    bars.append(
        plt.bar(xs[i], ys[i], width=bar_width, label=labels[i], hatch=h, ec="black",lw=1, color=colors[i], log=False))

    plt.text(xs[i][5] - 0.3 * bar_width, ys[i][5] + 150,int(ys[i][5]),rotation=90, fontsize=12)


# 画x刻度
plt.xticks(tickx, labelx, rotation=0, fontsize=14)
plt.yticks([0,500,1000,1500,2000,2500], fontsize=14)
plt.xlim(-0.3,7)
# plt.ylim(0, 6)
plt.ylabel('KQPS', fontsize=14, fontweight='bold')
# plt.xlabel('Thread number',fontsize=14)
# plt.legend((bars[0], bars[3], bars[1], bars[4], bars[2], bars[5]),
#            (labels[0], labels[3], labels[1], labels[4], labels[2], labels[5]), loc=4, fontsize=14, ncol=3,
#            bbox_to_anchor=(1, 0.9), borderpad=False, framealpha=0, labelspacing=0.1, columnspacing=0.5,
#            handletextpad=0.5)
plt.legend(ncol=2, fontsize=14, borderpad=False,
           framealpha=1, labelspacing=0.1, columnspacing=0.5,
           handletextpad=0.5,loc=4,bbox_to_anchor=(1, 0.6))
leg = plt.gca().get_legend()
ltext = leg.get_texts()
plt.setp(ltext, fontsize=14, fontweight='bold')  # 设置图例字体的大小和粗细

fig.tight_layout()
plt.savefig('./exp-kvell-ycsb.pdf', format='pdf')
plt.show()
