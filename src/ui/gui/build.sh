if pyrcc5 resources/resources.qrc -o resources_rc.py; then
	printf "pyrcc5 ran correctly \n"
else
	printf "something went wrong compiling python resource file."
fi

if pyuic5 mainwindow.ui -o mainwindow.py; then
	printf "pyuic5 ran correctly -- editing mainwindow.py\n"
	if sed -i 's/import resources_rc/from .resources_rc import qInitResources/g' mainwindow.py; then
		printf "edit success!\n"
	else
		printf "something went wrong when running sed"
	fi

else
	printf "something went wrong running pyuic5"
fi
