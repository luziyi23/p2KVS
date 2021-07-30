# coding=utf-8

import numpy as np
import matplotlib.pyplot as plt
import xlrd

excel = xlrd.open_workbook("moti-fio-threads.xlsx")
sheet = excel.sheet_by_index(0)
labels = ["Optane SSD-4KB", "Samsung SSD-4KB","Optane SSD-512B", "Samsung SSD-512B"]
ys = []
xs = []
xticks = sheet.row_values(0)[1:]
xticks=[int(x) for x in xticks]
for i in range(1, 5):
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
    xs.append(tempx)
    ys.append(temp)
plt.rcParams['font.sans-serif'] = ['Arial']  # 如果要显示中文字体,则在此处设为：SimHei
plt.rcParams['axes.unicode_minus'] = False  # 显示负号
print(xs)
print(ys)
# x = np.array(range(1, 301))

# label在图示(legend)中显示。若为数学公式,则最好在字符串前后添加"$"符号
# color：b:blue、g:green、r:red、c:cyan、m:magenta、y:yellow、k:black、w:white、、、
# 线型：-  --   -.  :    ,
# marker：.  ,   o   v    <    *    +    1
fig = plt.figure(figsize=(4, 2))
# plt.figure()
plt.grid(linestyle="--", axis="y")  # 设置背景网格线为虚线
ax = plt.gca()
ax.spines['top'].set_visible(False)  # 去掉上边框
ax.spines['right'].set_visible(False)  # 去掉右边框

plt.plot(xs[0], ys[0], marker='o',markersize=4, label=labels[0], linewidth=1, linestyle='-', color="black")
plt.plot(xs[1], ys[1], marker='x',markersize=8, label=labels[1], linewidth=1, linestyle='-', color="black")
plt.plot(xs[2], ys[2], marker='o',markersize=4, label=labels[2], linewidth=1, linestyle='--', color="blue")
plt.plot(xs[3], ys[3], marker='x',markersize=8, label=labels[3], linewidth=1, linestyle='--', color="blue")
# plt.plot(xs[2], ys[2], marker='', label=labels[2], linewidth=2, linestyle='-')#, color="red")
# plt.plot(xs[3], ys[3], marker='', label=labels[3], linewidth=2, linestyle='-')#, color="orange")

plt.xticks(xs[0], xticks, fontsize=12, rotation=0)  # 默认字体大小为10
plt.yticks([500, 1000, 1500, 2000, 2500], fontsize=10)
# plt.minorticks_off()
# plt.title("example", fontsize=12, fontweight='bold')  # 默认字体大小为12
# plt.xlabel("Reading Threads Number", fontsize=20, fontweight='bold')
# plt.xlabel("Thread Number", fontsize=12, fontweight='bold')
# plt.ylabel("Latency(ms)", fontsize=16, fontweight='bold')
plt.ylabel("IO Bandwidth(MB/s)", fontsize=10, fontweight='bold')
# plt.xlim(0, 100)  # 设置x轴的范围
plt.ylim(0, 2500)
# plt.legend()          #显示各曲线的图例
ax.legend(loc=4, numpoints=1, ncol=2, fontsize=10, borderpad=False, framealpha=0, bbox_to_anchor=(1, 1),columnspacing=0.5,handletextpad=0.5)
# leg = ax.get_legend()
# ltext = leg.get_texts()
# plt.setp(ltext, fontsize=12)  # 设置图例字体的大小和粗细
fig.tight_layout()
plt.savefig('./moti-fio-threads.pdf', format='pdf')  # 建议保存为svg格式,再用inkscape转为矢量图emf后插入word中
plt.show()
