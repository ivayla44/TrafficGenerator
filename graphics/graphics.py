import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget
import pyqtgraph as pg
import json


class StatsGraph(QMainWindow):
    def __init__(self, stats):
        super().__init__()

        self.setWindowTitle('Stats Graph')
        self.setGeometry(100, 100, 1200, 600)

        self.stats = stats

        self.centralWidget = QWidget()
        self.setCentralWidget(self.centralWidget)
        self.layout = QVBoxLayout(self.centralWidget)

        self.plot()

    def plot(self):
        # Plot for rx bytes and tx bytes
        self.graphWidget1 = pg.PlotWidget()
        self.layout.addWidget(self.graphWidget1)

        self.graphWidget1.setBackground('w')
        self.graphWidget1.setTitle('RX Bytes and TX Bytes')
        self.graphWidget1.setLabel('left', 'Values')
        self.graphWidget1.setLabel('bottom', 'Stats')

        # Filter stats for rx bytes and tx bytes
        rx_tx_stats = {key: value for key, value in self.stats.items() if key in ['cnt_rx_bytes', 'cnt_tx_bytes']}
        x1 = range(len(rx_tx_stats))
        labels1 = list(rx_tx_stats.keys())
        y1 = list(rx_tx_stats.values())

        self.graphWidget1.setXRange(-0.5, len(rx_tx_stats) - 0.5, padding=0.1)

        # Plot bars for rx bytes and tx bytes (purple color)
        bar1 = pg.BarGraphItem(x=x1, height=y1, width=0.6, brush=pg.mkColor((148, 0, 211)))  # Purple
        self.graphWidget1.addItem(bar1)
        self.graphWidget1.getPlotItem().getAxis('bottom').setTicks([list(zip(x1, labels1))])

        # Plot for other statistics
        self.graphWidget2 = pg.PlotWidget()
        self.layout.addWidget(self.graphWidget2)

        self.graphWidget2.setBackground('w')
        self.graphWidget2.setTitle('Packet Statistics')
        self.graphWidget2.setLabel('left', 'Values')
        self.graphWidget2.setLabel('bottom', 'Stats')

        # Filter stats for other statistics
        packet_stats = {key: value for key, value in self.stats.items() if key not in ['cnt_rx_bytes', 'cnt_tx_bytes']}
        x2 = range(len(packet_stats))
        labels2 = list(packet_stats.keys())
        y2 = list(packet_stats.values())

        self.graphWidget2.setXRange(-0.5, len(packet_stats) - 0.5, padding=0.1)

        # Plot bars for other statistics (orange color)
        bar2 = pg.BarGraphItem(x=x2, height=y2, width=0.6, brush=pg.mkColor((255, 165, 0)))  # Orange
        self.graphWidget2.addItem(bar2)
        self.graphWidget2.getPlotItem().getAxis('bottom').setTicks([list(zip(x2, labels2))])


def read_stats_from_json(file_path):
    with open(file_path, 'r') as file:
        stats = json.load(file)
    return stats


if __name__ == '__main__':
    file_path = '../files/summary_stats.json'

    try:
        stats = read_stats_from_json(file_path)
    except FileNotFoundError:
        print(f"File '{file_path}' not found.")
        sys.exit()

    app = QApplication(sys.argv)
    window = StatsGraph(stats)
    window.show()
    sys.exit(app.exec_())
