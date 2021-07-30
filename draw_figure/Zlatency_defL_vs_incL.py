import numpy as np
import matplotlib.pyplot as plt
import xlrd

plt.rcParams['font.sans-serif'] = ['Arial']  # 如果要显示中文字体,则在此处设为：SimHei
plt.rcParams['axes.unicode_minus'] = False  # 显示负号
plt.rcParams['xtick.bottom'] = False

colors = ["#213C66", "#7ACABB", "#FFFFAA"]
excel = xlrd.open_workbook("latency_defL_vs_incL.xlsx")
sheet = excel.sheet_by_index(0)

# 准备图中数据,顺带把legend也准备了
labels = []
xs = []
ys = []
bar_width = 0.35
for i in range(1, sheet.nrows):
    row = sheet.row_values(i)
    temp = []
    tempx = []
    labels.append(row[0])
    for ind, j in enumerate(row[1:]):
        if ind % 2 == 0:
            ind += bar_width / 4
        tempx.append(ind)
        temp.append(j/1000)
    xs.append(tempx)
    ys.append(temp)
for ind, x in enumerate(xs):
    for i in range(len(x)):
        xs[ind][i] += ind * bar_width
print(labels)
print(xs)
print(ys)
# 准备x轴刻度字样和位置数据
models = list(sheet.row_values(0)[1:])
# workload = ["A", "B", "C", "D"]
labelx = []
tickx = []
indexes = xs[1]
for i in range(0, len(xs[0]), 2):
    tickx.append(indexes[i])
    # tickx.append(indexes[i] / 2 + indexes[i + 1] / 2)
    tickx.append(indexes[i + 1])
    labelx.append(models[i])
    # labelx.append("\n" + workload[round(i/2)])
    labelx.append(models[i + 1])
print(tickx)
print(labelx)

# 建图
fig = plt.figure(figsize=(4, 2.5))
gca = plt.gca()
gca.set_axisbelow(True)
gca.grid(axis='y', linestyle='dotted', lw=1, alpha=1)
gca.spines['top'].set_visible(False)  # 去掉上边框
gca.spines['right'].set_visible(False)  # 去掉右边框

# 画柱子
for i in range(len(xs)):
    # for x in range(len(xs[i])):
    #     xs[i][x] += i * bar_width
    plt.bar(xs[i], ys[i], width=bar_width, label=labels[i], ec="black", color=colors[i], log=True)

# 画x刻度
plt.xticks(tickx, labelx, rotation=90, fontsize=10)
plt.yticks(fontsize=11)
plt.minorticks_off()
plt.ylabel('Latency(μs)', fontsize=12)
plt.xlabel("LOAD   A     C", fontsize=12, fontweight="bold")

# 其他
plt.legend(loc=4, fontsize=10, ncol=2, bbox_to_anchor=(1, 1.05), borderpad=False, framealpha=0)
leg = plt.gca().get_legend()
ltext = leg.get_texts()
plt.setp(ltext, fontsize=12, fontweight='bold')  # 设置图例字体的大小和粗细
plt.ylim(0, 4000)
plt.xlim(-0.2, 8.2)
fig.tight_layout()
plt.savefig('./latency_defL_vs_incL.pdf', format='pdf')
plt.show()
