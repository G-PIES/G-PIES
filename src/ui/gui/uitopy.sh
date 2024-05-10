if pyuic5 mainwindow.ui -o mainwindow.py; then
	printf "pyuic5 ran correctly -- editing mainwindow.py\n"
	if sed -i 's/import icon_reasource_rc/from .icon_reasource_rc import qInitResources/g' mainwindow.py; then
		printf "edit success!\n"
	else
		printf "something went wrong when running sed"
	fi

else
	printf "something went wrong running pyuic5"
fi
