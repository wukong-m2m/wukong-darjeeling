# Wukong Master server

This repository contains the source code for Wukong Master server, a python tornado server for Python 2.7+

# Prerequisites

To run the server the following must be present in your system:

Python modules:

* lxml					- a fast xml parser
* tornado				- a scalable, non-blocking web server
* pyzwave			- a python module to communicate with zwave devices through a basestation

Install using [easy_install](http://pypi.python.org/pypi/setuptools) run:

    sudo easy_install lxml tornado

Or using [pip](http://pypi.python.org/pypi/pip/)(You can install pip by `sudo easy_install pip`) run:

    sudo pip install lxml tornado

Pyzwave is custom built, so it cannot be obtained from public repository.

	cd {NanoKong}/tools/python/pyzwave
	sudo python setup.py install

# Running

The server is always on port 5000, but it can be changed easily in the source code.

To run the server:

	python master_server.py [IP]
	
IP is the ip the server will bind to, so if you want to host a public server, you can bind it to a public IP, therefore it can be seen and access from the outside, otherwise omitting the parameter will default to "localhost"

# Structure

To get more information about internal workings of tornado server, please refer to [Tornado](http://www.tornadoweb.org/)

As the time of writing, we are planning for Tornado to be a user interface to Wukong master node to manage, modify, deploy applications. Including, not limited to, the ability to create applications in flow based programming style by dragging components and link them together.

## Routes

Tornado server look a lot like Google's webapp structure. All the routes are specified in application constructor in tuple (r"/route/for/something", function_name) inside an array.

## Main window

![main](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/main.png)

### Structure

The main window consists of a list of existing application entries and a button on the left bar to add a new application. Each application entry has 3 buttons on the right side of the bar. 

The first one from the left is monitor, which is used to monitor deployed application's status. The second one from the left is deploy, which is used to deploy applications with application created by the application editor. The last one is delete, which is used to delete and destroy all files related to this application.

In order to let user interaction to feel more fluid, we have implemented a pure ajax based web application by using javascript to switch out and in the elements if necessary without refreshing the page at all, therefore creating this really fast transitions between screens without having to wait for the browser to finish the loading bar.

When user adds an application, master_server.py will receive a ajax POST request from the browser client, and application will create a new Application object with an id, a md5 hash from application name to minimize collision, a Wukong Application xml string, a pointer to local application directory on local hard drive (makefile, etc), a pointer to the compiler thread and a pointer to the deployer thread. Once the application is created, a new directory will be created on the local harddrive which is a copy from a folder called 'base' in the same parent directory with the new application. The applications folder is located at [`NanoKong/vm/apps/`](https://github.com/wukong-m2m/NanoKong/tree/WukongMaster/vm/apps)

It goes the same for deleting, monitoring, or deploying the application as they all have corresponding routes on the Tornado server side.

## Application editor

![fbp](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/editor.png)

* Update application: /application/[1-fA-F\d]{32} [PUT]

Application editor could be accessed by double clicking the empty area of an application entry from the main window. Application editor has two components; the one on the top is the configuration of the application such as application name, description, and possibly compilation configurations; the bottom area is filled with a canvas element for programming the application.

In our initial design, we have made application configuration form takes a different route from the FBP application editor, so there are one save button for each component. But we plan to integrate them into one route, or we could implement a more real time saving of the configuration without user hitting the save button twice.

### FBP.js (Flow Based Programming)

Flow based programming is a popular programming paradigm for people who don't have knowledge of how programming works with general purpose languages like c/c++, java, python, etc. It works by dragging and dropping to create components that could connect with other components. If the users need a Temperature component, all he/she needs is to select the right component from the dropdown menu and hit create button. To connect two components together a user selects a component and hit connect and select another component, and then a dialog will appear to select the properties to connect with the component with.

User can save his/her creation by hitting the save button

## Deploy window

![deploy](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/deploy.png)

![deploy-logic](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/deploy-logic.png)

After a valid fbp application has been drawn, a user can deploy this application logic to any physical Wukong devices in this screen. At first there is only the xml manifestation of the fbp application, we need to "map" appropriate components to appropriate devices. This mapping process is been done in our Wukong Mapper which is located in translator.py under `{NanoKong}/tools/python`

Once the mapping algorithm has produced the mapping results, the user can now "deploy" the application along with the mapping results in application code form onto the Wukong devices. This works under the hood by calling the rule to translator application xml to working code in whatever language we supported at the moment (right now only java), and we will convert it into a bytecode format contained in file named nvmdefault.h which will be transmitted to the devices to start the reprogramming process.

## Monitor window

### Monitor logging

![monitor-logging](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/monitor-logging.png)

### Monitor mapping

![monitor-mapping](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/monitor-mapping.png)

### Monitor properties

![monitor-properties](https://raw.github.com/wukong-m2m/NanoKong/gh-pages/images/monitor-properties.png)