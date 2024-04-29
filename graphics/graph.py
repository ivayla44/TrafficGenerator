import json
import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget, QLabel
import pyqtgraph as pg
import numpy as np


class GraphWindow(QMainWindow):
    def __init__(self, data):
        super().__init__()
        self.data = data
        self.setWindowTitle("Transmitted Packets per Second")
        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        layout = QVBoxLayout(self.central_widget)
        self.plot_widget = pg.PlotWidget()
        layout.addWidget(self.plot_widget)
        self.total_pps_label = QLabel()
        layout.addWidget(self.total_pps_label)
        self.set_plot_data()

    def set_plot_data(self):
        detailed_entries = self.data['detailed']
        tx_packets = [entry['cnt_pkts'] for entry in detailed_entries]
        durations = [entry['duration'] for entry in detailed_entries]
        packets_per_second = [tx / duration for tx, duration in zip(tx_packets, durations)]

        total_pps = sum(tx / duration * tx for tx, duration in zip(tx_packets, durations))
        self.total_pps_label.setText(f"Total PPS: {total_pps}")

        time_seconds = np.arange(0, len(packets_per_second) * 60, 60)

        self.plot_widget.clear()
        self.plot_widget.plot(time_seconds, packets_per_second, pen='b', symbol='o', symbolPen='b', symbolBrush='b')
        self.plot_widget.setLabel('left', 'Packets per Second')
        self.plot_widget.setLabel('bottom', 'Time (seconds)')
        self.plot_widget.setTitle('Transmitted Packets per Second')
        self.plot_widget.showGrid(x=True, y=True)


def main():
    with open('../files/summary_stats.json', 'r') as file:
        data = json.load(file)

    app = QApplication(sys.argv)
    window = GraphWindow(data)
    window.show()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
