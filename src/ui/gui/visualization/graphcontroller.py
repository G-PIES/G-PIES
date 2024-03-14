from gui.visualization.mplcanvas import MplCanvas


class GraphController:
    def __init__(self, parent_layout, width=5, height=4, dpi=100):
        self.parent_layout = parent_layout
        self.width = width
        self.height = height
        self.dpi = dpi
        self.scatter_plot = None

    def init_graph(self, C):
        self.lines = {}
        self.data_by_c = {}
        # Prevent duplicate graph creation on subsequent simulations
        if self.scatter_plot:
            self.parent_layout.removeWidget(self.scatter_plot)
            self.scatter_plot.deleteLater()
            self.scatter_plot = None

        self.scatter_plot = MplCanvas(width=self.width, height=self.height, dpi=self.dpi)
        self.data_by_c = {i: {'x': [], 'y': []} for i in range(1, C)}
        for c_value in self.data_by_c:
            self.lines[c_value], = self.scatter_plot.axes.plot([], [], label=f'C: {c_value}')

        self.scatter_plot.axes.set_xlabel('Time (s)')
        self.scatter_plot.axes.set_ylabel('Interstitials / cm^3')
        self.parent_layout.addWidget(self.scatter_plot)
        self.scatter_plot.show()

    def update_graph(self, data_accumulator):
        # Replace graphed data with current DataAccumulator data
        self.data_by_c.clear()
        for block in data_accumulator.data:
            for entry in block:
                t = entry[0]
                c = int(entry[1])
                y = entry[2]
                if c not in self.data_by_c:
                    self.data_by_c[c] = {'x': [], 'y': []}
                self.data_by_c[c]['x'].append(t)
                self.data_by_c[c]['y'].append(y)

        for c_value, line in self.lines.items():
            data = self.data_by_c[c_value]
            line.set_data(data['x'], data['y'])

        self.scatter_plot.axes.relim()
        self.scatter_plot.axes.autoscale_view(True, True, True)
        self.scatter_plot.figure.canvas.draw_idle()

    def get_legend_items(self):
        return {c_value: (line.get_label(), line.get_color()) for c_value, line in self.lines.items()}
