# coding=utf-8

import numpy as np
import matplotlib.pyplot as plt
import xlrd
from matplotlib import gridspec
plt.rcParams['font.sans-serif'] = ['Arial']  # 如果要显示中文字体,则在此处设为：SimHei
plt.rcParams['axes.unicode_minus'] = False  # 显示负号
plt.rcParams['xtick.bottom'] = False
# colors = ["#F0F0F0","#DD7B00",  "#358582"]
colors = ["#140032","#358582", "#7ACABB", "#FFFFAA"]
excel=xlrd.open_workbook("bg-total.xlsx")
sheet= excel.sheet_by_index(0)
labels=[]
ys=[]
xs=[]
index=[]
bar_width=0.25
#数据
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
labelx = list(sheet.row_values(0)[1:])
tickx=np.array(xs[1])
print(tickx)
print(labelx)
#建图
fig = plt.figure(figsize=(8, 2.5))
gs = gridspec.GridSpec(1, 3, width_ratios=[3, 1,1])
ax1=plt.subplot(gs[0])
gca = plt.gca()
gca.set_axisbelow(True)
gca.grid(axis='y', linestyle='dotted', lw=1, alpha=1)
gca.spines['top'].set_visible(False)  # 去掉上边框
gca.spines['right'].set_visible(False)  # 去掉右边框

#画柱子
for i in range(len(xs)):
    # for x in range(len(xs[i])):
    #     xs[i][x] += i * bar_width
    plt.bar(xs[i][:3], ys[i][:3], width=bar_width, label=labels[i], ec="black", color=colors[i],log=False)
# plt.text(xs[0][-1]-bar_width*1.5,2050,"3707")
#画x刻度
plt.xticks(tickx[:3], labelx[:3], rotation=0, fontsize=12)
plt.yticks(fontsize=12)
plt.xlim(-0.25,2.75)
plt.ylabel('KQPS',fontsize=14, fontweight='bold')
plt.legend(fontsize=10,borderpad=False,framealpha=1)
leg = plt.gca().get_legend()
ltext = leg.get_texts()
plt.setp(ltext, fontsize=12, fontweight='bold')  # 设置图例字体的大小和粗细


ax2=plt.subplot(gs[1])
#画柱子
for i in range(len(xs)):
    plt.bar(xs[i][3:4], ys[i][3:4], width=bar_width, label=labels[i], ec="black", color=colors[i],log=False)
#画x刻度
plt.xticks(tickx[3:4], labelx[3:4], rotation=0, fontsize=12)
plt.yticks(fontsize=12)
plt.xlim(2.75,3.75)
ax2.set_axisbelow(True)
ax2.grid(axis='y', linestyle='dotted', lw=1, alpha=1)
ax2.spines['top'].set_visible(False)  # 去掉上边框
ax2.spines['right'].set_visible(False)  # 去掉右边框



ax3=plt.subplot(gs[2])
#画柱子
for i in range(len(xs)):
    plt.bar(xs[i][4:], ys[i][4:], width=bar_width, label=labels[i], ec="black", color=colors[i],log=False)
#画x刻度
plt.xticks(tickx[4:], labelx[4:], rotation=0, fontsize=12)
plt.yticks(fontsize=12)
plt.xlim(3.75,4.75)
#其他
ax3.set_axisbelow(True)
ax3.grid(axis='y', linestyle='dotted', lw=1, alpha=1)
ax3.spines['top'].set_visible(False)  # 去掉上边框
ax3.spines['right'].set_visible(False)  # 去掉右边框
# plt.legend(loc=1,fontsize=8,ncol=4,bbox_to_anchor=(0.81, 1),borderpad=False,framealpha=0)
# plt.ylim(0,2000)
fig.tight_layout()
plt.savefig('./bg-total.pdf', format='pdf')
plt.show()