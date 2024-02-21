from PyQt5.Qt import QApplication
from gui.mainwindowcontroller import MainWindowController


app = QApplication([])
window = MainWindowController()
window.show()
app.exec_()
