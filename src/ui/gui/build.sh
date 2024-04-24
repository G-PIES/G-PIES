printf "Running pyrcc5 .. compiling resources --"
if pyrcc5 resources/resources.qrc -o resources_rc.py; then
	printf " success!\n"
else
	printf " something went wrong running pyrcc5.\n"
fi

printf "Running pyuic5 .. compiling ui file --"
if pyuic5 mainwindow.ui -o mainwindow.py; then
	printf " success!\n"
	printf "Editing py file to correct import --"
	if sed -i 's/import resources_rc/from .resources_rc import qInitResources/g' mainwindow.py; then
		printf " edit success!\n"
	else
		printf " something went wrong when running sed.\n"
	fi

else
	printf " something went wrong running pyuic5.\n"
fi
