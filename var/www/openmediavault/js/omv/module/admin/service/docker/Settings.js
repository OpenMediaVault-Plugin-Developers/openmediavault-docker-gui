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

// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")
// require("js/omv/module/admin/service/docker/ImageGrid.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/form/field/SharedFolderComboBox.js")
// require("js/omvextras/window/RootFolderBrowser.js")
// require("js/omv/window/MessageBox.js")
// require("js/omv/Rpc.js")

Ext.define("OMV.module.admin.service.docker.Settings", {
    extend: "OMV.workspace.form.Panel",

    rpcService: "Docker",
    rpcGetMethod: "getSettings",
    rpcSetMethod: "setSettings",

    initComponent : function() {
        this.on("load", function () {
            var me = this;
            var parent = this.up("tabpanel");

            if (!parent) {
                return;
            }

            var overviewPanel = parent.down("panel[title=" + _("Overview") + "]");
            var settingsPanel = parent.down("panel[title=" + _("Settings") + "]");
            var repoPanel = parent.down("panel[title=" + _("Docker images repo") + "]");
            var networksPanel = parent.down("panel[title=" + _("Networks") + "]");
            var dockerVersion = settingsPanel.findField("version").getValue();
            var checked = settingsPanel.findField("enabled").checked

            if (overviewPanel) {
                if (checked) {
                    overviewPanel.tab.show();
                    overviewPanel.enable();
                    overviewPanel.down("dockerImageGrid").doReload();
                    overviewPanel.down("dockerContainerGrid").doReload();
                    repoPanel.tab.show();
                    repoPanel.enable();
                    repoPanel.doReload();
                    networksPanel.tab.show();
                    networksPanel.enable();
                    networksPanel.doReload();
                    parent.setActiveTab(overviewPanel);
                } else {
                    overviewPanel.disable();
                    overviewPanel.tab.hide();
                    repoPanel.disable();
                    repoPanel.tab.hide();
                    networksPanel.disable();
                    networksPanel.tab.hide();
                    OMV.Rpc.request({
                        scope: me,
                        callback: function(id, success, response) {
                        },
                        relayErrors: false,
                        rpcData: {
                            service: "Docker",
                            method: "syncDockerLogos",
                        }
                    });
                }
                if (dockerVersion === "0") {
                    settingsPanel.findField("enabled").setDisabled(true);
                  } else {
                    settingsPanel.findField("enabled").setDisabled(false);
                }
            }

        }, this);

        this.callParent(arguments);
    },

    getFormItems: function() {
        var me = this;
        return [{
            xtype: "fieldset",
            title: _("General"),
            fieldDefaults: {
                labelSeparator: ""
            },
            items: [{
                xtype: "checkbox",
                name: "enabled",
                boxLabel: _("Enable the plugin"),
                plugins: [{
                    ptype: "fieldinfo",
                    text: _("This setting does not stop/disable or start/enable the docker daemon")
                }]
            },{
                xtype: "checkbox",
                name: "cwarn",
                boxLabel: _("Warn when modifying container")
            },{
                xtype: "sharedfoldercombo",
                name: "sharedfolderref",
                plugins: [{
                    ptype: "fieldinfo",
                    text: _("The location of the Docker base path (this setting is optional and defaults to /var/lib/docker if unset). The plugin must be enabled for a change to be committed")
                }],
                allowNone: true,
                allowBlank: true
            }]
        },{
            xtype: "fieldset",
            title: _("Information"),
            fieldDefaults: {
                labelSeparator: ""
            },
            items: [{
                fieldLabel: "Version",
                xtype: "textareafield",
                name: "dockerVersion",
                editable: false,
                cls: Ext.baseCSSPrefix + "form-textarea-monospaced",
                submitValue: false,
                grow: true
            },{
                fieldLabel: "Info",
                xtype: "textareafield",
                name: "dockerInfo",
                editable: false,
                cls: Ext.baseCSSPrefix + "form-textarea-monospaced",
                submitValue: false,
                grow: true
            }]
        },{
            xtype: "hiddenfield",
            name: "version"
        }];
    }

});

OMV.WorkspaceManager.registerPanel({
    id: "settings",
    path: "/service/docker",
    text: _("Settings"),
    position: 20,
    className: "OMV.module.admin.service.docker.Settings"
});
