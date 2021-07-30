# coding=utf-8

import numpy as np
import matplotlib.pyplot as plt
import xlrd

excel = xlrd.open_workbook("exp-avg-latency.xlsx")
sheet = excel.sheet_by_index(0)
labels = sheet.col_values(0)[1:]
xs = sheet.row_values(0)[1:]
# xs = [x*1000 for x in xs]
print(xs)
ys = []

max_y = []
avg_y = []
for i in range(1, 4):
    row = sheet.row_values(i)
    temp = []
    tempx = []
    for ind, j in enumerate(row[1:]):
        if j == '':
            continue
        tempx.append(ind)
        temp.append(j)
        # if len(temp)==500:
        #     break
    ys.append(temp)
    max_y.append(max(temp))
    avg_y.append(np.mean(temp))
print(ys)
plt.rcParams['font.sans-serif'] = ['Arial']  # 如果要显示中文字体,则在此处设为：SimHei
plt.rcParams['axes.unicode_minus'] = False  # 显示负号

# label在图示(legend)中显示。若为数学公式,则最好在字符串前后添加"$"符号
# color：b:blue、g:green、r:red、c:cyan、m:magenta、y:yellow、k:black、w:white、、、
# 线型：-  --   -.  :    ,
# marker：.  ,   o   v    <    *    +    1
fig = plt.figure(figsize=(4, 2.5))
# plt.figure()
plt.grid(linestyle="--", axis="y")  # 设置背景网格线为虚线
ax = plt.gca()
ax.spines['top'].set_visible(False)  # 去掉上边框
ax.spines['right'].set_visible(False)  # 去掉右边框
line1 = plt.loglog(xs, ys[0], marker='o', markersize=5, label=labels[0], linewidth=1.5, linestyle='-', color="green")
line2 = plt.loglog(xs, ys[1], marker='o', markersize=5, label=labels[1], linewidth=1.5, linestyle='-', color="orange")
line3 = plt.loglog(xs, ys[2], marker='o', markersize=6, label=labels[2], linewidth=2.5, linestyle='-', color="darkred")
# plt.semilogy(xs[3], ys[3], marker='', label=labels[3], linewidth=1.5, linestyle='-')#, color="orange")
# plt.semilogy(xs[4], ys[4], marker='', label=labels[4], linewidth=1.0, linestyle='-')#, color="black")
# plt.plot(xs, ys[0], marker='', label=labels[0], linewidth=3, linestyle='-', color="blue")
# plt.plot(xs, ys[1], marker='', label=labels[1], linewidth=3, linestyle='-', color="green")
# plt.plot(xs[2], ys[2], marker='', label=labels[2], linewidth=2, linestyle='-')#, color="red")
# plt.plot(xs[3], ys[3], marker='', label=labels[3], linewidth=2, linestyle='-')#, color="orange")
# plt.hlines(2400, 0, 100, linestyles='--', colors="black", label="SSD bandwidth", linewidth=2)
# plt.hlines(np.mean(ys[1][:100]), 0, 100, colors="orange", linestyles='--', label="Average throughput of random write",
#            linewidth=2)

plt.xticks(fontsize=12)  # 默认字体大小为10
plt.yticks(fontsize=12)
# plt.minorticks_off()
# plt.title("example", fontsize=12, fontweight='bold')  # 默认字体大小为12
# plt.xlabel("Reading Threads Number", fontsize=20, fontweight='bold')
plt.xlabel("$\\times1000$ requests per second", fontsize=12, fontweight='bold')
# plt.ylabel("Latency(ms)", fontsize=16, fontweight='bold')
plt.ylabel(r'Latency($\mu$s)', fontsize=12, fontweight='bold')
# plt.xlim(0, 100)  # 设置x轴的范围
# plt.ylim(0, 2500)
# plt.yticks(fontsize=12)
# plt.legend()          #显示各曲线的图例
ax.legend(loc=4, numpoints=1, ncol=2, fontsize=10, borderpad=False, framealpha=0, bbox_to_anchor=(0.9, 0.95),
          labelspacing=0.1, columnspacing=1, handletextpad=0.1)
# ax.legend(numpoints=1, ncol=1, fontsize=10, borderpad=False, framealpha=1, labelspacing=0.1, columnspacing=0.1, handletextpad=0.1)
leg = ax.get_legend()
ltext = leg.get_texts()
plt.setp(ltext, fontsize=12, fontweight='bold')  # 设置图例字体的大小和粗细
fig.tight_layout()
plt.savefig('./exp-avg-latency.pdf', format='pdf')  # 建议保存为svg格式,再用inkscape转为矢量图emf后插入word中
plt.show()
