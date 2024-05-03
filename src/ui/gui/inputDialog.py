
from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import QDialog, QDialogButtonBox, QFormLayout

#So this file is where I'm currently putting some stuff that was getting overwritten in mainwindow.py during pyuic5

class InputDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)

        self.reactor_temp=QtWidgets.QLineEdit(self)
        self.reactor_flux=QtWidgets.QLineEdit(self)
        self.reactor_recombination=QtWidgets.QLineEdit(self)
        self.reactor_i_bi=QtWidgets.QLineEdit(self)
        self.reactor_i_tri=QtWidgets.QLineEdit(self)
        self.reactor_i_quad=QtWidgets.QLineEdit(self)
        self.reactor_dislocation_density_evolution=QtWidgets.QLineEdit(self)
        
        
        self.atomic_volume=QtWidgets.QLineEdit(self)
        self.atomic_i_migration=QtWidgets.QLineEdit(self)
        self.atomic_i_diffusion=QtWidgets.QLineEdit(self)
        self.atomic_i_formation=QtWidgets.QLineEdit(self)
        self.atomic_i_binding=QtWidgets.QLineEdit(self)
        self.atomic_recombination_radius=QtWidgets.QLineEdit(self)
        self.atomic_i_loop_bias=QtWidgets.QLineEdit(self)
        self.atomic_i_dislocation_bias=QtWidgets.QLineEdit(self)
        self.atomic_i_dislocation_bias_param=QtWidgets.QLineEdit(self)
        self.atomic_dislocation_density=QtWidgets.QLineEdit(self)
        self.atomic_grain_size=QtWidgets.QLineEdit(self)
        self.atomic_lattice_param=QtWidgets.QLineEdit(self)
        self.atomic_burgurs_vector=QtWidgets.QLineEdit(self)
            
        self.widget = QtWidgets.QWidget(self)
        self.layout = QFormLayout(self.widget)
    
    def getReactorSettings(self):

        # Create a new QWidget to hold the layout
        for i in reversed(range(self.layout.count())): 
            self.layout.itemAt(i).widget().setParent(None)
        
        first = QtWidgets.QLineEdit()
        second = QtWidgets.QLineEdit()
        third=QtWidgets.QLineEdit()
        fourth=QtWidgets.QLineEdit()
        fifth=QtWidgets.QLineEdit()
        sixth=QtWidgets.QLineEdit()
        seventh=QtWidgets.QLineEdit()
        
        self.layout.addRow("Set Temperature", first)
        self.layout.addRow("Set Flux", second)
        self.layout.addRow("Set Recombination", third)
        self.layout.addRow("Set I_Bi", fourth)
        self.layout.addRow("Set I_Tri", fifth)
        self.layout.addRow("Set I_Quad", sixth)
        self.layout.addRow("Set Dislocation Density", seventh)
        
        buttonBox = QtWidgets.QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        
        self.reactor_temp=first
        self.reactor_flux=second
        self.reactor_recombination=third
        self.reactor_i_bi=fourth
        self.reactor_i_tri=fifth
        self.reactor_i_quad=sixth
        self.reactor_dislocation_density_evolution=seventh
        self.layout.addWidget(buttonBox)
        buttonBox.accepted.connect(self.accept)
        buttonBox.rejected.connect(self.reject)

    
        self.setLayout(self.layout)
        #self.layout.addRow("Set Atomic Volume", second)

    def getMaterialSettings(self):
        for i in reversed(range(self.layout.count())): 
            self.layout.itemAt(i).widget().setParent(None)
        
        first = QtWidgets.QLineEdit()
        second = QtWidgets.QLineEdit()
        third=QtWidgets.QLineEdit()
        fourth=QtWidgets.QLineEdit()
        fifth=QtWidgets.QLineEdit()
        sixth=QtWidgets.QLineEdit()
        seventh=QtWidgets.QLineEdit()
        eighth=QtWidgets.QLineEdit()
        ninth=QtWidgets.QLineEdit()
        tenth=QtWidgets.QLineEdit()
        eleventh=QtWidgets.QLineEdit()
        twelfth=QtWidgets.QLineEdit()
        thirteenth=QtWidgets.QLineEdit()
        self.layout.addRow("Set Atomic Size", first)
        self.layout.addRow("Set I_Migration", second)
        self.layout.addRow("Set I_Diffision", third)
        self.layout.addRow("Set I_Formation", fourth)
        self.layout.addRow("Set I_Binding", fifth)
        self.layout.addRow("Set Recombination Radius", sixth)
        self.layout.addRow("Set I_Loop Bias", seventh)
        self.layout.addRow("Set Dislocation Bias", eighth)
        self.layout.addRow("Set Dislocation Bias Param", ninth)
        self.layout.addRow("Set Dislocation Density", tenth)
        self.layout.addRow("Set Grain Size", eleventh)
        self.layout.addRow("Set Lattice Param", twelfth)
        self.layout.addRow("Set Burgur's Vector", thirteenth)
        buttonBox = QtWidgets.QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        
        self.atomic_volume=first
        self.atomic_i_migration=second
        self.atomic_i_diffusion=third
        self.atomic_i_formation=fourth
        self.atomic_i_binding=fifth
        self.atomic_recombination_radius=sixth
        self.atomic_i_loop_bias=seventh
        self.atomic_i_dislocation_bias=eighth
        self.atomic_i_dislocation_bias_param=ninth
        self.atomic_dislocation_density=tenth
        self.atomic_grain_size=eleventh
        self.atomic_lattice_param=twelfth
        self.atomic_burgurs_vector=thirteenth
        
        
        self.layout.addWidget(buttonBox)
        buttonBox.accepted.connect(self.accept)
        buttonBox.rejected.connect(self.reject)

        self.setLayout(self.layout) 
        
    def getReactorInputs(self):
        return (self.reactor_temp.text(), self.reactor_flux.text(),self.reactor_recombination.text()
                ,self.reactor_i_bi.text(), self.reactor_i_tri.text(), self.reactor_i_quad.text(), 
                self.reactor_dislocation_density_evolution.text())
        
    def getMaterialInputs(self):
        return(self.atomic_volume.text(), self.atomic_i_migration.text(), self.atomic_i_diffusion.text(), 
               self.atomic_i_formation.text(), self.atomic_i_binding.text(), self.atomic_recombination_radius.text(), 
               self.atomic_i_loop_bias.text(), self.atomic_i_dislocation_bias.text(), self.atomic_i_dislocation_bias_param.text(), 
               self.atomic_dislocation_density.text(), self.atomic_grain_size.text(), self.atomic_lattice_param.text(), 
               self.atomic_burgurs_vector.text(), self.atomic_)
    
