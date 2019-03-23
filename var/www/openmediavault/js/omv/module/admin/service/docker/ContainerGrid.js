/**
 * Copyright (c) 2015-2019 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// require("js/omv/module/admin/service/docker/PortRow.js")
// require("js/omv/module/admin/service/docker/EnvVarRow.js")
// require("js/omv/module/admin/service/docker/BindMountRow.js")
// require("js/omv/module/admin/service/docker/RunContainer.js")
// require("js/omv/module/admin/service/docker/CreateContainer.js")
// require("js/omv/module/admin/service/docker/ExecuteCmd.js")
// require("js/omv/module/admin/service/docker/DetailsDlg.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

Ext.define("OMV.module.admin.service.docker.ContainerGrid", {
    extend: "OMV.workspace.grid.Panel",
    alias: "widget.dockerContainerGrid",
    plugins: "gridfilters",

    id: "dockerContainerGrid",
    disableDeleteButton: true,
    deleteButtonText: _("Delete"),

    rpcService: "Docker",
    rpcGetMethod: "getContainers",
    requires: [
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc",
        "OMV.module.admin.service.docker.DetailsDlg"
    ],

    stateful: true,
    stateId: "280eb3e7-c505-449c-9c84-1eb1b62b6b6a",

    columns: [{
        xtype: "textcolumn",
        text: _("CONTAINER ID"),
        dataIndex: 'id',
        sortable: true,
        stateId: 'id',
    },{
        xtype: "textcolumn",
        text: _("IMAGE"),
        dataIndex: 'image',
        sortable: true,
        stateId: 'image',
        filter: 'string'
    },{
        xtype: "textcolumn",
        text: _("COMMAND"),
        dataIndex: 'command',
        sortable: true,
        stateId: 'command'
    },{
        xtype: "textcolumn",
        text: _("CREATED"),
        dataIndex: 'created',
        sortable: true,
        stateId: 'created'
    },{
        xtype: "textcolumn",
        text: _("STATUS"),
        dataIndex: 'status',
        sortable: true,
        stateId: 'status'
    },{
        xtype: "textcolumn",
        text: _("PORTS"),
        dataIndex: 'ports',
        sortable: true,
        stateId: 'ports'
    },{
        xtype: "textcolumn",
        text: _("NAMES"),
        dataIndex: 'name',
        sortable: true,
        stateId: 'name'
    },{
        xtype: "textcolumn",
        text: _("STATE"),
        dataIndex: 'state',
        sortable: true,
        stateId: 'state',
        filter: 'list'
    },{
        xtype: "binaryunitcolumn",
        text: _("LOG SIZE"),
        dataIndex: 'logsize',
        sortable: true,
        stateId: 'logsize',
        filter: 'list'
    },{
        xtype: "textcolumn",
        text: _("EXTRA ARGUMENTS"),
        dataIndex: 'extraargs',
        sortable: false,
        stateId: 'extraargs',
        filter: 'string'
    },{
        xtype: "textcolumn",
        text: _("CONTAINER COMMAND"),
        dataIndex: 'containercommand',
        sortable: false,
        stateId: 'containercommand',
        filter: 'string'
    }],

    initComponent: function() {
        var me = this;
        Ext.apply(me, {
            store: Ext.create("OMV.data.Store", {
                autoLoad: true,
                model: OMV.data.Model.createImplicit({
                    fields: [
                        { name: "id", type: "string" },
                        { name: "image", type: "string" },
                        { name: "command", type: "string" },
                        { name: "created", type: "string" },
                        { name: "status", type: "string" },
                        { name: "state", type: "string" },
                        { name: "ports", type: "string" },
                        { name: "networkmode", type: "string" },
                        { name: "macvlan_ipaddress", type: "string"},
                        { name: "macvlan_network", type: "string"},
                        { name: "restartpolicy", type: "string" },
                        { name: "maxretries", type: "integer"},
                        { name: "privileged", type: "boolean" },
                        { name: "exposedports", type: "array" },
                        { name: "envvars", type: "array" },
                        { name: "cenvvars", type: "array" },
                        { name: "portbindings", type: "array" },
                        { name: "name", type: "string" },
                        { name: "volumesfrom", type: "array" },
                        { name: "extraargs", type: "string"},
                        { name: "hostname", type: "string" },
                        { name: "timesync", type: "boolean" },
                        { name: "imagevolumes", type: "array" },
                        { name: "containercommand", type: "string" },
                        { name: "logsize", type: "integer" }
                    ]
                }),
                proxy: {
                    type: "rpc",
                    rpcData: {
                        service: "Docker",
                        method: "getContainers",
                    }
                }
            })
        });
        me.callParent(arguments);
    },

    getTopToolbarItems: function(c) {
        var me = this;
        return [{
            id: me.getId() + "-create",
            xtype: "button",
            text: _("Create"),
            icon: "images/add.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled: true,
            handler: Ext.Function.bind(me.onCreateButton, me, [ me ]),
            scope: me
        },{
            xtype: "splitbutton",
            text: "Start",
            icon: "images/play.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            scope: me,
            menu: Ext.create("Ext.menu.Menu", {
                items: [{
                    id: me.getId() + "-start",
                    text: _("Start"),
                    disabled: true,
                    iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
                    icon: "images/play.png",
                    handler: Ext.Function.bind(me.onStartButton, me, [ me ]),
                    scope: me
                },{
                    id: me.getId() + "-stop",
                    text: _("Stop"),
                    disabled: true,
                    iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
                    icon: "images/docker_stop.png",
                    handler: Ext.Function.bind(me.onStopButton, me, [ me ]),
                    scope: me
                },{
                    id: me.getId() + "-restart",
                    text: _("Restart"),
                    disabled: true,
                    iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
                    icon: "images/undo.png",
                    handler: Ext.Function.bind(me.onRestartButton, me, [ me ]),
                    scope: me
                }]
            })
        },{
            id: me.getId() + "-copy",
            xtype: "button",
            text: _("Copy"),
            icon: "images/docker_copy.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled: true,
            handler: Ext.Function.bind(me.onCopyButton, me, [ me ]),
            scope: me
        },{
            id: me.getId() + "-modify",
            xtype: "button",
            text: _("Modify"),
            icon: "images/edit.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled: true,
            handler: Ext.Function.bind(me.onModifyButton, me, [ me ]),
            scope: me
        },{
            id: me.getId() + "-details",
            xtype: "button",
            text: _("Details"),
            icon: "images/search.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled: true,
            handler: Ext.Function.bind(me.onDetailsButton, me, [ me ]),
            scope: me
        },{
            id: me.getId() + "-logs",
            xtype: "button",
            text: _("Logs"),
            icon: "images/logs.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled: true,
            handler: Ext.Function.bind(me.onLogsButton, me, [ me ]),
            scope: me
        },{
            id: me.getId() + "-execute",
            xtype: "button",
            text: _("Run cmd"),
            icon: "images/docker_exec.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled: true,
            handler: Ext.Function.bind(me.onExecuteButton, me, [ me ]),
            scope: me
        },{
            id: me.getId() + "-commit",
            xtype: "button",
            text: _("Commit"),
            icon: "images/save.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled: true,
            handler: Ext.Function.bind(me.onCommitButton, me, [ me ]),
            scope: me
        },{
            id: me.getId() + "-delete",
            xtype: "button",
            text: _("Delete"),
            icon: "images/delete.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled: true,
            handler: Ext.Function.bind(me.onDeleteButton, me, [ me ]),
            scope: me
        },{
            id: me.getId() + "-refresh",
            xtype: "button",
            text: _("Refresh"),
            icon: "images/refresh.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            hidden: false,
            handler: Ext.Function.bind(me.onRefreshButton, me, [ me ]),
            scope: me
        },{
            id: me.getId() + "-clearlog",
            xtype: "button",
            text: _("Clear Log"),
            icon: "images/trashcan.png",
            iconCls: Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled: true,
            hidden: false,
            handler: Ext.Function.bind(me.onClearLogButton, me, [ me ]),
            scope: me
        }]
    },

    onSelectionChange: function(model, records) {
        var me = this;
        if(me.hideTopToolbar)
            return;
        var tbarBtnName = [ "create", "start", "stop", "restart", "copy", "modify", "details", "execute", "commit", "logs", "delete", "refresh", "clearlog" ];
        var tbarBtnDisabled = {
            "create": false,
            "start": false,
            "stop": false,
            "restart": false,
            "copy": false,
            "modify": false,
            "details": false,
            "logs": false,
            "execute": false,
            "commit": false,
            "logs": false,
            "delete": false,
            "refresh": false,
            "clearlog": false
        };
        // Enable/disable buttons depending on the number of selected rows.
        if(records.length <= 0) {
            tbarBtnDisabled["start"] = true;
            tbarBtnDisabled["stop"] = true;
            tbarBtnDisabled["restart"] = true;
            tbarBtnDisabled["copy"] = true;
            tbarBtnDisabled["modify"] = true;
            tbarBtnDisabled["details"] = true;
            tbarBtnDisabled["logs"] = true;
            tbarBtnDisabled["execute"] = true;
            tbarBtnDisabled["commit"] = true;
            tbarBtnDisabled["delete"] = true;
        } else if(records.length == 1) {
            // Disable 'Delete' button if selected node is not stopped
            Ext.Array.each(records, function(record) {
                if(!(record.get("state") === "dead" || record.get("state") === "stopped")) {
                    tbarBtnDisabled["delete"] = true;
                    return false;
                }
            });

            // Disable 'Execute' and 'Logs' buttons if selected node is not running
            Ext.Array.each(records, function(record) {
                if(!(record.get("state") === "running")) {
                    tbarBtnDisabled["execute"] = true;
                    tbarBtnDisabled["logs"] = true;
                    return false;
                }
            });

            // Disable buttons if selected node is a data container
            Ext.Array.each(records, function(record) {
                if(record.get("status") === "Created") {
                    tbarBtnDisabled["start"] = true;
                    tbarBtnDisabled["stop"] = true;
                    tbarBtnDisabled["restart"] = true;
                    tbarBtnDisabled["execute"] = true;
                    tbarBtnDisabled["logs"] = true;
                    return false;
                }
            });
        } else {
            tbarBtnDisabled["copy"] = true;
            tbarBtnDisabled["modify"] = true;
            tbarBtnDisabled["details"] = true;
            tbarBtnDisabled["commit"] = true;
            tbarBtnDisabled["logs"] = true;
            // Disable 'Delete' button if selected nodes are not stopped
            Ext.Array.each(records, function(record) {
                if(!(record.get("state") === "dead" || record.get("state") === "stopped")) {
                    tbarBtnDisabled["delete"] = true;
                    return false;
                }
            });

            // Disable buttons if selected node is a data container
            Ext.Array.each(records, function(record) {
                if(record.get("status") === "Created") {
                    tbarBtnDisabled["start"] = true;
                    tbarBtnDisabled["stop"] = true;
                    tbarBtnDisabled["restart"] = true;
                    tbarBtnDisabled["execute"] = true;
                    return false;
                }
            });
        }

        // Update the button controls.
        Ext.Array.each(tbarBtnName, function(name) {
            var tbarBtnCtrl = me.queryById(me.getId() + "-" + name);
            if(!Ext.isEmpty(tbarBtnCtrl)) {
                if(true == tbarBtnDisabled[name]) {
                    tbarBtnCtrl.disable();
                } else {
                    tbarBtnCtrl.enable();
                }
            }
        });
    },

    onCreateButton: function() {
        var me = this;
        var imageStore = me.up('tabpanel').down('dockerImageGrid').getStore();
        Ext.create("OMV.module.admin.service.docker.CreateContainer", {
            title: _("Create data container"),
            imageStore: imageStore
        }).show();
    },

    onStartButton: function() {
        var me = this;
        var sm = me.getSelectionModel();
        var selRecords = sm.getSelection();
        var record = selRecords[0];
        var index;
        var idList = "";
        for(i = 0; i < selRecords.length; i++) {
            if(i === 0) {
                idList = selRecords[i].get("id");
            } else {
                idList = idList + " " + selRecords[i].get("id");
            }
        }
        OMV.Rpc.request({
            scope: me,
            callback: function(id, success, response) {
                sm.deselectAll();
                me.store.load({
                    scope: me,
                    callback: function(records, operation, success) {
                        for(i = 0; i < selRecords.length; i++) {
                            index = me.store.find('name', selRecords[i].get("name"));
                            sm.select(index, true);
                        }
                    }
                });
            },
            relayErrors: false,
            rpcData: {
                service: "Docker",
                method: "startContainer",
                params: {
                    id: idList
                }
            }
        });
    },

    onStopButton: function() {
        var me = this;
        var sm = me.getSelectionModel();
        var selRecords = sm.getSelection();
        var record = selRecords[0];
        var index;
        var idList = "";
        for(i = 0; i < selRecords.length; i++) {
            if(i === 0) {
                idList = selRecords[i].get("id");
            } else {
                idList = idList + " " + selRecords[i].get("id");
            }
        }
        OMV.Rpc.request({
            scope: me,
            callback: function(id, success, response) {
                sm.deselectAll();
                me.store.load({
                    scope: me,
                    callback: function(records, operation, success) {
                        for(i = 0; i < selRecords.length; i++) {
                            index = me.store.find('name', selRecords[i].get("name"));
                            sm.select(index, true);
                        }
                    }
                });
            },
            relayErrors: false,
            rpcData: {
                service: "Docker",
                method: "stopContainer",
                params: {
                    id: idList
                }
            }
        });
    },

    onRestartButton: function() {
        var me = this;
        var sm = me.getSelectionModel();
        var selRecords = sm.getSelection();
        var record = selRecords[0];
        var index;
        var idList = "";
        for(i = 0; i < selRecords.length; i++) {
            if(i === 0) {
                idList = selRecords[i].get("id");
            } else {
                idList = idList + " " + selRecords[i].get("id");
            }
        }
        OMV.Rpc.request({
            scope: me,
            callback: function(id, success, response) {
                sm.deselectAll();
                me.store.load({
                    scope: me,
                    callback: function(records, operation, success) {
                        for(i = 0; i < selRecords.length; i++) {
                            index = me.store.find('name', selRecords[i].get("name"));
                            sm.select(index, true);
                        }
                    }
                });
            },
            relayErrors: false,
            rpcData: {
                service: "Docker",
                method: "restartContainer",
                params: {
                    id: idList
                }
            }
        });
    },

    onClearLogButton: function() {
        var me = this;
        var sm = me.getSelectionModel();
        var selRecords = sm.getSelection();
        var record = selRecords[0];
        var index;
        var idList = "";
        for(i = 0; i < selRecords.length; i++) {
            if(i === 0) {
                idList = selRecords[i].get("id");
            } else {
                idList = idList + " " + selRecords[i].get("id");
            }
        }
        OMV.Rpc.request({
            scope: me,
            callback: function(id, success, response) {
                sm.deselectAll();
                me.store.load({
                    scope: me,
                    callback: function(records, operation, success) {
                        for(i = 0; i < selRecords.length; i++) {
                            index = me.store.find('name', selRecords[i].get("name"));
                            sm.select(index, true);
                        }
                    }
                });
            },
            relayErrors: false,
            rpcData: {
                service: "Docker",
                method: "clearLog",
                params: {
                    id: idList
                }
            }
        });
    },

    onDetailsButton: function() {
        var me = this;
        var record = me.getSelected();

        Ext.create("OMV.module.admin.service.docker.DetailsDlg", {
            title: _("Container details"),
            rpcGetParams: {
                id: record.get('id')
            }
        }).show();
    },

    onLogsButton: function() {
        var me = this;
        var sm = me.getSelectionModel();
        var records = sm.getSelection();
        var record = records[0];

        var detailsWindow = Ext.create("OMV.workspace.window.Form", {
            title: _("Container logs"),
            rpcService: "Docker",
            rpcGetMethod: "getLogs",
            rpcGetParams: {
                id: record.get('id')
            },
            width: 1100,
            height: 700,
            hideResetButton: true,
            hideCancelButton: true,
            okButtonText: _("Close"),
            scrollable: false,

            getFormItems: function() {
                var me = this;

                return [{
                    xtype: "textarea",
                    name: "logs",
                    grow: false,
                    editable: false,
                    height: 620,
                    cls: "x-form-textarea-monospaced"
                }];
            }
        }).show();
    },

    onExecuteButton : function() {
        var me = this;
        var sm = me.getSelectionModel();
        var records = sm.getSelection();
        var record = records[0];
        Ext.create("OMV.module.admin.service.docker.ExecuteCmd", {
            title: _("Execute command"),
            rpcService: "Docker",
            rpcMethod: "executeCommand",
            hideStopButton: true,
            containerId: record.get("id"),
            listeners: {
                scope: me,
                exception: function(wnd, error) {
                    OMV.MessageBox.error(null, error);
                }
            }
        }).show();
    },

    onCommitButton : function() {
        var me = this;
        var sm = me.getSelectionModel();
        var records = sm.getSelection();
        var record = records[0];
        var newImage = "";
        newImage = prompt("Enter name of the new image", "");
        OMV.Rpc.request({
            scope: me,
            callback: function(id, success, response) {
                Ext.getCmp("dockerImageGrid").doReload();
            },
            relayErrors: false,
            rpcData: {
                service: "Docker",
                method: "commitContainer",
                params: {
                    name: record.get("name"),
                    newImage: newImage
                }
            }
        });
    },

    onCopyButton: function() {
        var me = this;
        var sm = me.getSelectionModel();
        var records = sm.getSelection();
        var record = records[0];
        OMV.Rpc.request({
            scope: me,
            callback: function(id, success, response) {
                if (success && response) {
                    if (record.get("status") === "Created") {
                        var imageStore = me.up('tabpanel').down('dockerImageGrid').getStore();
                        Ext.create("OMV.module.admin.service.docker.CreateContainer", {
                            title: _("Copy data container"),
                            imageStore: imageStore,
                            image: response["image"],
                            bindmounts: response["bindmounts"]
                        }).show();
                    } else {
                        Ext.create("OMV.module.admin.service.docker.RunContainer", {
                            title: _("Copy container"),
                            image: response["image"],
                            ports: response["exposedports"],
                            envvars: response["envvars"],
                            restartpolicy: response["restartpolicy"],
                            maxretries: response["maxretries"],
                            privileged: response["privileged"],
                            networkmode: response["networkmode"],
                            macvlan_network: response["macvlan_network"],
                            macvlan_ipaddress: response["macvlan_ipaddress"],
                            portbindings: response["portbindings"],
                            bindmounts: response["bindmounts"],
                            cenvvars: response["cenvvars"],
                            copyVolumes: response["volumesfrom"],
                            extraargs: response["extraargs"],
                            hostname: response["hostname"],
                            timesync: response["timesync"],
                            imagevolumes: response["imagevolumes"],
                            containercommand: response["containercommand"]
                        }).show();
                    }
                }
            },
            relayErrors: false,
            rpcData: {
                service: "Docker",
                method: "getContainerData",
                params: {
                    id: record.get("id")
                }
            }
        });
    },

    onModifyButton: function() {
        var me = this;
        var sm = me.getSelectionModel();
        var records = sm.getSelection();
        var record = records[0];
        if (record.get("status") === "Created") {
            OMV.MessageBox.show({
                title: _("Modify container"),
                msg: _("It is not possible to modify a data container."),
                scope: me,
                buttons: Ext.Msg.OK
            });

        } else {
            OMV.Rpc.request({
                scope: me,
                callback: function(id, success, response) {
                    if (response) {
                        //Display warning if setting is enabled
                        OMV.MessageBox.show({
                            title: _("Warning"),
                            msg: _("Please be aware that <b>all</b> non-persistent data within the container</br>" +
                                   "will be <b>deleted</b></br>" +
                                   "Please see here for more information:</br>" +
                                   "<a href='http://forums.openmediavault.org/index.php/Thread/10921-openmediavault-docker-gui-Testing/?postID=101996#post102450'" +
                            " target='_blank'>Link</a></br>" +
                                "This warning can be disabled on the Settings tab"),
                            scope: me,
                            buttons: Ext.Msg.OK,
                            fn: function(buttonId) {
                                if (buttonId === "ok") {
                                    OMV.Rpc.request({
                                        scope: me,
                                        callback: function(id, success, response) {
                                            if (success && response) {
                                                Ext.create("OMV.module.admin.service.docker.RunContainer", {
                                                    title: _("Modify container"),
                                                    image: response["image"],
                                                    ports: response["exposedports"],
                                                    envvars: response["envvars"],
                                                    restartpolicy: response["restartpolicy"],
                                                    maxretries: response["maxretries"],
                                                    privileged: response["privileged"],
                                                    networkmode: response["networkmode"],
                                                    macvlan_ipaddress: response["macvlan_ipaddress"],
                                                    macvlan_network: response["macvlan_network"],
                                                    portbindings: response["portbindings"],
                                                    bindmounts: response["bindmounts"],
                                                    cenvvars: response["cenvvars"],
                                                    copyVolumes: response["volumesfrom"],
                                                    hostname: response["hostname"],
                                                    timesync: response["timesync"],
                                                    imagevolumes: response["imagevolumes"],
                                                    extraargs: response["extraargs"],
                                                    containercommand: response["containercommand"],
                                                    name: response["name"],
                                                    cid: response["id"],
                                                    action: "modify"
                                                }).show();
                                            }
                                        },
                                        relayErrors: false,
                                        rpcData: {
                                            service: "Docker",
                                            method: "getContainerData",
                                            params: {
                                                id: record.get("id")
                                            }
                                        }
                                    });

                                }
                            }
                        });
                    } else {
                        OMV.Rpc.request({
                            scope: me,
                            callback: function(id, success, response) {
                                if (success && response) {
                                    Ext.create("OMV.module.admin.service.docker.RunContainer", {
                                        title: _("Modify container"),
                                        image: response["image"],
                                        ports: response["exposedports"],
                                        envvars: response["envvars"],
                                        restartpolicy: response["restartpolicy"],
                                        maxretries: response["maxretries"],
                                        privileged: response["privileged"],
                                        networkmode: response["networkmode"],
                                        macvlan_network: response["macvlan_network"],
                                        macvlan_ipaddress: response["macvlan_ipaddress"],
                                        portbindings: response["portbindings"],
                                        bindmounts: response["bindmounts"],
                                        cenvvars: response["cenvvars"],
                                        copyVolumes: response["volumesfrom"],
                                        hostname: response["hostname"],
                                        extraargs: response["extraargs"],
                                        containercommand: response["containercommand"],
                                        timesync: response["timesync"],
                                        imagevolumes: response["imagevolumes"],
                                        name: response["name"],
                                        cid: response["id"],
                                        action: "modify"
                                    }).show();
                                }
                            },
                            relayErrors: false,
                            rpcData: {
                                service: "Docker",
                                method: "getContainerData",
                                params: {
                                    id: record.get("id")
                                }
                            }
                        });

                    }
                },
                relayErrors: false,
                rpcData: {
                    service: "Docker",
                    method: "modifyWarningEnabled",
                    params: {
                        cid: record.get("id")
                    }
                }
            });
        }
    },

    doDeletion: function(record) {
        var me = this;
        OMV.Rpc.request({
            scope: me,
            callback: me.onDeletion,
            rpcData: {
                service: "Docker",
                method: "deleteContainer",
                params: {
                    id: record.get('id')
                }
            }
        });
    }
});
