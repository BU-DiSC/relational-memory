# import sys
# !{sys.executable} -m pip install -q --upgrade ipywidgets matplotlib
import subprocess

import ipywidgets as widgets
from IPython.display import display
from matplotlib import pyplot as plt
from pygments import highlight
from pygments.formatters import HtmlFormatter
from pygments.lexers import SqlLexer

ip = '10.210.1.184'
data_types = {
    'TINYINT': 1,
    'SMALLINT': 2,
    'INT': 4,
    'BIGINT': 8
}

margin = {'margin': '0 10px'}


def single_bar_plot(ax, data):
    for key, value in data.items():
        ax.bar(key, value)


def ssh_run(desc):
    args = [str(x) for x in desc]
    print(' '.join(['ssh', ip, './query'] + args))
    result = subprocess.run(['ssh', ip, './query'] + args, stdout=subprocess.PIPE, text=True)
    try:
        return int(result.stdout)
    except ValueError:
        print('Output error')
        print(result.stdout)
        return 0


def format_sql(stmt):
    return highlight(stmt, SqlLexer(), HtmlFormatter())


def update_dropdown(self, table, drop):
    if len(drop.options) != len(table.cols.children):
        old_value = drop.value
        drop.options = [col.description for col in table.cols.children]
        drop.value = 'c0' if old_value not in drop.options else old_value
        self.update_sql()


def bar_plot(title, x, data, xlabel=''):
    # Set the width of the bars
    bar_width = 0.2

    # Set the positions of the bars on the x-axis

    bar_positions1 = list(range(len(x)))
    bar_positions2 = [x + bar_width for x in bar_positions1]
    bar_positions3 = [x + bar_width for x in bar_positions2]
    bar_positions = [bar_positions1, bar_positions2, bar_positions3]

    # Plot the bars for each dataset
    for (key, value), positions in zip(data.items(), bar_positions):
        plt.bar(positions, value, bar_width, label=key)

    # Set the labels and title
    if xlabel:
        plt.xlabel(xlabel)
    # ax.set_ylabel('Y')
    plt.title(title)

    # Set the x-axis tick positions and labels
    plt.xticks(bar_positions2, x)

    # Add a legend
    plt.legend()


class Benchmark:
    def __init__(self):
        self.execute_button = widgets.Button(description='Execute', icon='play')
        self.execute_button.on_click(self.execute)

        self.plot = widgets.Output()
        self.title = 'Benchmark'
        self.widget = widgets.VBox([self.execute_button, self.plot])

    def fig7(self):
        x = [1, 2, 4, 8, 16]
        data = {
            'ROW': [n + 2 for n in x],
            'COL': [n + 1 for n in x],
            'RME': x
        }
        plt.subplot(4, 2, 1)
        bar_plot('Projectivity (3 columns)', x, data, xlabel='Column width in Bytes')

    def fig8(self):
        x = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
        data = {
            'ROW': [n + 2 for n in x],
            'COL': [n + 1 for n in x],
            'RME': x
        }
        plt.subplot(4, 2, 2)
        bar_plot('Varying Projectivity', x, data, xlabel='Number of target columns')

    def fig12a(self):
        x = [1, 2, 4, 8, 16]
        data = {
            'ROW': [n + 2 for n in x],
            'COL': [n + 1 for n in x],
            'RME': x
        }
        plt.subplot(4, 2, 3)
        bar_plot('Join', x, data, xlabel='Column width in Bytes')

    def fig12b(self):
        x = [16, 32, 64, 128, 256]
        data = {
            'ROW': [n + 2 for n in x],
            'COL': [n + 1 for n in x],
            'RME': x
        }
        plt.subplot(4, 2, 4)
        bar_plot('Join', x, data, xlabel='Row width in Bytes')

    def fig13a(self):
        x = [9, 18, 35, 69, 137, 273, 545]
        data = {
            'ROW': [n + 2 for n in x],
            'COL': [n + 1 for n in x],
            'RME': x
        }
        plt.subplot(4, 2, 5)
        bar_plot('fig 13a', x, data)

    def fig13b(self):
        x = [11, 22, 44, 87, 173, 346, 692]
        data = {
            'ROW': [n + 2 for n in x],
            'COL': [n + 1 for n in x],
            'RME': x
        }
        plt.subplot(4, 2, 6)
        bar_plot('fig 13b', x, data)

    def fig14(self):
        x = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
        data = {
            'ROW': [n + 2 for n in x],
            'COL': [n + 1 for n in x],
            'RME': x
        }
        plt.subplot(4, 2, 7)
        bar_plot('Projectivity & MVCC', x, data, xlabel='Number of target columns')

    def draw(self):
        self.plot.clear_output()
        plt.figure(figsize=(16, 24))
        self.fig7()
        self.fig8()
        self.fig12a()
        self.fig12b()
        self.fig13a()
        self.fig13b()
        self.fig14()
        with self.plot:
            plt.show()

    def execute(self, _=None):
        self.execute_button.disabled = True
        self.draw()
        self.execute_button.disabled = False


class Demo:
    def __init__(self, components):
        self.components = components

        self.experiments = widgets.HBox([widgets.Checkbox(value=True, description=exp, indent=False, layout={'width': 'auto'}) for exp in ('ROW', 'COL', 'RME')])
        self.execute_button = widgets.Button(description='Execute', icon='play')
        self.execute_button.on_click(self.execute)
        self.plot = widgets.Output(layout={'width': '400px', 'height': '300px'})

        self.stmt_stack = widgets.Stack([c.stmts for c in components[:-1]], selected_index=0)
        self.stmt = widgets.VBox([self.experiments, self.execute_button, self.stmt_stack], layout={'width': '230px', 'margin': '0 10px'})

        self.widget = widgets.Tab([self.make_view(c) if i != 3 else c.widget for (i, c) in enumerate(components)])
        self.widget.titles = [c.title for c in components]

        def update_view(change):
            if change.new != 3:
                component = components[change.new]
                self.stmt_stack.selected_index = change.new
                component.update()
                self.draw()

        self.widget.observe(update_view, names='selected_index')
        self.draw()

    def make_view(self, component):
        view = widgets.HBox([component.widget, self.stmt, self.plot])
        return view

    def draw(self):
        component = self.components[self.widget.selected_index]
        self.plot.clear_output()
        fig, ax = plt.subplots(figsize=(4, 3))
        single_bar_plot(ax, component.data)
        plt.ylabel('Execution time (μs)')
        with self.plot:
            plt.show()

    def execute(self, _=None):
        component = self.components[self.widget.selected_index]
        component.data = {'ROW': 0, 'COL': 0, 'RME': 0}
        self.execute_button.disabled = True
        for store in self.experiments.children:
            if store.value:
                component.execute(store.description)
                self.draw()
        self.execute_button.disabled = False


class Table:
    def __init__(self, name):
        self.components = []
        self.stmt = widgets.HTML()

        self.name = name

        self.cols = widgets.VBox()
        self.rows = widgets.BoundedIntText(value=32768, max=4294967295, description='Rows:')
        self.add_button = widgets.Button(description='Add column', icon='plus')
        self.add_button.on_click(self.add_column)
        self.remove_button = widgets.Button(description='Remove column', icon='trash')
        self.remove_button.on_click(self.remove_column)
        buttons = widgets.HBox([self.add_button, self.remove_button])
        label = widgets.Label(f'Create {name}:')
        self.widget = widgets.VBox([label, buttons, self.rows, self.cols])

        self.add_column()

    def update_sql(self):
        attrs = ', \n  '.join(map(lambda t: ' '.join(t), ((attr.description, attr.value) for attr in self.cols.children)))
        self.stmt.value = format_sql(f'CREATE TABLE {self.name} (\n  {attrs}\n);')

    def update(self, _=None):
        self.update_sql()
        self.remove_button.disabled = len(self.cols.children) == 1
        # self.add_button.disabled = len(self.cols.children) == 10
        for c in self.components:
            c.update()

    def add_column(self, _=None):
        drop = widgets.Dropdown(options=data_types.keys(), value='INT', description=f'c{len(self.cols.children)}')
        drop.observe(self.update, names='value')
        self.cols.children = self.cols.children + (drop,)
        self.update()

    def remove_column(self, _=None):
        self.cols.children = self.cols.children[:-1]
        self.update()


class Aggregate:
    def __init__(self, stable):
        self.title = 'Aggregation'
        self.data = {'ROW': 0, 'COL': 0, 'RME': 0}

        self.stmt = widgets.HTML()
        self.stmts = widgets.VBox([stable.stmt, self.stmt], layout={'width': '100%'})

        self.stable = stable

        self.col = widgets.Dropdown(description='column')
        self.col.observe(self.update_sql, names='value')
        tables = widgets.VBox([stable.widget], layout=margin)
        knobs = widgets.VBox([widgets.Label('Average:'), self.col], layout=margin)
        self.widget = widgets.HBox([tables, knobs])

        self.update()

    def update_sql(self, _=None):
        self.stmt.value = format_sql(f'SELECT AVG({self.col.value}) FROM {self.stable.name};')

    def execute(self, store):
        s_widths = [str(data_types[col.value]) for col in self.stable.cols.children]
        desc = [store, 'q1', self.stable.rows.value, len(s_widths), ','.join(s_widths), self.col.index]
        result = ssh_run(desc)
        self.data[store] = result
        return self.data[store]

    def update(self, _=None):
        update_dropdown(self, self.stable, self.col)


class SPW(widgets.HBox):
    def __init__(self, x):
        super().__init__()
        self.label = widgets.Label(x.description, layout={'width': '20px', 'margin': '0 8px'})
        self.select = widgets.Checkbox(indent=False, layout={'width': '33px'})
        self.check = widgets.Checkbox(indent=False, layout={'width': '18px'})
        self.k = widgets.BoundedIntText(value=2 ** (data_types[x.value] * 8 - 1),
                                        max=2 ** (min(data_types[x.value], 6) * 8) - 1,
                                        disabled=True, layout={'width': '205px'})
        self.check.observe(self.toggle, names='value')
        self.children = self.label, self.select, self.check, self.k

    def toggle(self, _=None):
        self.k.disabled = not self.check.value


class SelectionProjection:
    def __init__(self, stable):
        self.title = 'Projection & Selection'
        self.data = {'ROW': 0, 'COL': 0, 'RME': 0}

        self.stmt = widgets.HTML()
        self.stmts = widgets.VBox([stable.stmt, self.stmt], layout={'width': '100%'})

        self.stable = stable
        tables = widgets.VBox([stable.widget], layout=margin)

        self.cols = widgets.VBox(layout=margin)
        knobs = widgets.VBox([widgets.HBox([widgets.Label('Project:', layout={'margin': '2px 5px 2px 25px'}), widgets.Label('Select:')]), self.cols])
        self.widget = widgets.HBox([tables, knobs])

        self.update()

    def execute(self, store):
        s_widths = [str(data_types[col.value]) for col in self.stable.cols.children]
        cols = [f"{i}{'P' if spw.select.value else 'X'}{f'L{spw.k.value}' if spw.check.value else 'X'}"
                for i, spw in enumerate(self.cols.children) if spw.check.value or spw.select.value]
        desc = [store, 'q2', self.stable.rows.value, len(s_widths), ','.join(s_widths), len(cols), ','.join(cols)]
        result = ssh_run(desc)
        self.data[store] = result
        return self.data[store]

    def update_sql(self, _=None):
        selection = ', '.join(f'{spw.label.value}' for spw in self.cols.children if spw.select.value) or '*'
        filtering = '\n  AND '.join(f'{spw.label.value} < {spw.k.value}' for spw in self.cols.children if spw.check.value)
        where = f'\nWHERE {filtering}' if filtering else ''
        sql_code = f'SELECT {selection}\nFROM {self.stable.name}{where};'
        self.stmt.value = format_sql(sql_code)

    def update(self, _=None):
        if len(self.cols.children) < len(self.stable.cols.children):
            extra = tuple(SPW(x) for x in self.stable.cols.children[len(self.cols.children):])
            for spw in extra:
                spw.select.observe(self.update_sql, names='value')
            self.cols.children = self.cols.children + extra
        elif len(self.cols.children) > len(self.stable.cols.children):
            self.cols.children = self.cols.children[:len(self.stable.cols.children)]
        for child, col in zip(self.cols.children, self.stable.cols.children):
            label, select, check, k = child.children
            check.observe(self.update_sql, names='value')
            k.observe(self.update_sql, names='value')
            k.max = 2 ** (min(data_types[col.value], 6) * 8) - 1
        self.update_sql()


class Join:
    def __init__(self, stable, rtable):
        self.title = 'Join'
        self.data = {'ROW': 0, 'COL': 0, 'RME': 0}

        self.stmt = widgets.HTML()
        self.stmts = widgets.VBox([stable.stmt, rtable.stmt, self.stmt], layout={'width': '100%'})

        self.stable = stable
        self.rtable = rtable

        self.ssel = widgets.Dropdown(description=f'select {stable.name}')
        self.ssel.observe(self.update_sql, names='value')
        self.sjoin = widgets.Dropdown(description=f'join {stable.name}')
        self.sjoin.observe(self.update_sql, names='value')
        self.rsel = widgets.Dropdown(description=f'select {rtable.name}')
        self.rsel.observe(self.update_sql, names='value')
        self.rjoin = widgets.Dropdown(description=f'join {rtable.name}')
        self.rjoin.observe(self.update_sql, names='value')
        tables = widgets.VBox([stable.widget, rtable.widget], layout=margin)
        knobs = widgets.VBox([widgets.Label('Join:'), self.ssel, self.rsel, self.sjoin, self.rjoin], layout=margin)
        self.widget = widgets.HBox([tables, knobs])

        self.update()

    def execute(self, store):
        s_widths = [str(data_types[col.value]) for col in self.stable.cols.children]
        r_widths = [str(data_types[col.value]) for col in self.rtable.cols.children]
        desc = [store, 'q3', self.stable.rows.value, len(s_widths), ','.join(s_widths),
                self.rtable.rows.value, len(r_widths), ','.join(r_widths),
                self.ssel.index, self.rsel.index, self.sjoin.index, self.rjoin.index,]
        result = ssh_run(desc)
        self.data[store] = result
        return self.data[store]

    def update_sql(self, _=None):
        sql_code = f'SELECT {self.stable.name}.{self.ssel.value}, {self.rtable.name}.{self.rsel.value}\nFROM {self.stable.name}, {self.rtable.name}\nWHERE {self.stable.name}.{self.sjoin.value} = {self.rtable.name}.{self.rjoin.value};'
        self.stmt.value = format_sql(sql_code)

    def update(self, _=None):
        update_dropdown(self, self.stable, self.ssel)
        update_dropdown(self, self.stable, self.sjoin)
        update_dropdown(self, self.rtable, self.rsel)
        update_dropdown(self, self.rtable, self.rjoin)


def main():
    stable = Table('S')
    rtable = Table('R')
    aggregate = Aggregate(stable)
    sel_proj = SelectionProjection(stable)
    join = Join(stable, rtable)
    bench = Benchmark()

    components = (aggregate, sel_proj, join, bench)
    stable.components = rtable.components = components[:-1]

    display(widgets.HTML('''
    <style>
    :root {
      --jp-widgets-horizontal-tab-width: 160px;
    }
    </style>
    '''))
    demo = Demo(components)
    display(demo.widget)


main()
