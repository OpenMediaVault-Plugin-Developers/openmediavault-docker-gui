
<?php
/**
 * Copyright (c) 2015-2017 OpenMediaVault Plugin Developers
 *
 * @category OMVModuleDockerContainer
 * @package  Openmediavault-docker-gui
 * @author   OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @license  http://www.gnu.org/copyleft/gpl.html GNU General Public License
 * @link     https://github.com/OpenMediaVault-Plugin-Developers/openmediavault-docker-gui
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

require_once "Exception.php";
require_once "Utils.php";

/**
 * Class for managing Docker containers
 *
 * @category Class
 * @package  Openmediavault-docker-gui
 * @author   OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @license  http://www.gnu.org/copyleft/gpl.html GNU General Public License
 * @link     https://github.com/OpenMediaVault-Plugin-Developers/openmediavault-docker-gui
 *
 */

class OMVModuleDockerContainer
{
    // Attributes

    /**
     * Name of container image
     *
     * @var    string $_image
     * @access private
     */
    private $_image;

    /**
     * Id of the conatiner
     *
     * @var    string $_id
     * @access private
     */
    private $_id;

    /**
     * Time when container was created
     *
     * @var     string $_created
     * @access private
     */
    private $_created;

    /**
     * Command that is started by the container
     *
     * @var     string $_command
     * @access private
     */
    private $_command;

    /**
     * Status of the container
     *
     * @var     string $_status
     * @access private
     */
    private $_status;

    /**
     * State of the container
     *
     * @var     string $_state
     * @access private
     */
    private $_state;

    /**
     * Ports mapped to the container
     *
     * @var     array $_ports
     * @access private
     */
    private $_ports;

    /**
     * Network mode of the container
     *
     * @var     string $_networkMode
     * @access private
     */
    private $_networkMode;

    /**
     * Privileged mode of the container
     *
     * @var     bool $_privileged
     * @access private
     */
    private $_privileged;

    /**
     * Restart policy of the container
     *
     * @var     string $_restartPolicy
     * @access private
     */
    private $_restartPolicy;

    /**
     * Max number of retries for on-failure restart policy
     *
     * @var     string $_maxRetries
     * @access private
     */
    private $_maxRetries;

    /**
     * Environment variables defined in the container
     *
     * @var     array $_envVars
     * @access private
     */
    private $_envVars;

    /**
     * ID of the image the container is created from
     *
     * @var     string $_imageId
     * @access private
     */
    private $_imageId;

    /**
     * Port bindings in the container
     *
     * @var     array $_portBindings
     * @access private
     */
    private $_portBindings;

    /**
     * Bind mounts in the container
     *
     * @var     array $_bindMounts
     * @access private
     */
    private $_bindMounts;

    /**
     * Name of the container
     *
     * @var     string $_names
     * @access private
     */
    private $_names;

    /**
     * Does the container have any mountpoints
     *
     * @var     bool $_hasMounts
     * @access private
     */
    private $_hasMounts;

    /**
     * Array with containers used for volumes from
     *
     * @var array $_volumesFrom
     * @access private
     */
    private $_volumesFrom;

    /**
     * Host name of the container
     *
     * @var string $_hostName
     * @access private
     */
    private $_hostName;

    /**
     * Is the container syncing it's time with the host
     *
     * @var     bool $_timeSync
     * @access private
     */
    private $_timeSync;

    /**
     * Constructor. The container will be updated with all associated
     * properties from commandline.
     *
     * @param string $id      Id of the new image
     * @param array  $data    An associative array with all containers
     *
     * @return void
     * @access public
     */
    public function __construct($id, $data)
    {
        $this->_id = $id;
        $now = date("c");
        $item = $data[substr($id, 0, 12)];
        $this->_status = $item->Status;
        $this->_command = $item->Command;
        $this->_created = OMVModuleDockerUtil::getWhen(
            $now,
            date("c", $item->Created)
        ) . " ago";

        $url = "http::/containers/$id/json";
        $response = OMVModuleDockerUtil::doApiCall($url);
        $containerData = json_decode($response);
        $this->_image = $containerData->Config->Image;
        $this->_state = "running";
        if ($containerData->State->Running) {
            $this->_state = "running";
        } elseif (($containerData->State->Dead) || ($containerData->State->ExitCode !== 0)) {
            $this->_state = "dead";
        } elseif (($containerData->State->ExitCode === 0) && (strcmp($containerData->State->Error, "") === 0)) {
            $this->_state = "stopped";
        }
        $this->_ports = array();
        if (isset($containerData->NetworkSettings->Ports)) {
            foreach ($containerData->NetworkSettings->Ports as $exposedport => $hostports) {
                if ($hostports) {
                    $this->_ports[$exposedport] = array();
                    foreach ($hostports as $hostport) {
                        $tmparray = array(
                            "HostIp" => $hostport->HostIp,
                            "HostPort" => $hostport->HostPort);
                        array_push($this->_ports[$exposedport], $tmparray);
                    }
                } else {
                    $this->_ports[$exposedport] = null;
                }
            }
        }
        $this->_networkMode = $containerData->HostConfig->NetworkMode;

        $macvlans = OMVModuleDockerUtil::getMacVlanNetworks();
        $macvlans = array_column($macvlans,'name');
        $macvlan_network = $containerData->HostConfig->NetworkMode;
        if (strcmp($this->_networkMode, "default") === 0) {
            $this->_networkMode = "bridge";
        } elseif (in_array($macvlan_network, $macvlans)) {
            $this->_networkMode = "macvlan";
            $this->_macVlanContainerNetwork = $containerData->HostConfig->NetworkMode;
            $this->_macVlanContainerIpAddress = $containerData->NetworkSettings->Networks->$macvlan_network->IPAddress;
        }
        $this->_privileged = $containerData->HostConfig->Privileged;
        $this->_restartPolicy = $containerData->HostConfig->RestartPolicy->Name;
        $this->_maxRetries = $containerData->HostConfig->RestartPolicy->MaximumRetryCount;
        $this->_envVars = array();
        if (isset($containerData->Config->Env)) {
            foreach ($containerData->Config->Env as $eVar) {
                $eVarAry = explode("=", $eVar,2);
                $this->_envVars[$eVarAry[0]] = $eVarAry[1];
            }
        }
        $this->_imageId = substr($containerData->Image, 7);
        $this->_portBindings = array();
        if (isset($containerData->NetworkSettings->Ports)) {
            foreach ($containerData->NetworkSettings->Ports as $containerPort => $mappings) {
                foreach ($mappings as $mapping) {
                    array_push(
                        $this->_portBindings,
                        array(
                            "containerportstring" => $containerPort,
                            "containerportnr" => preg_split('/\//', $containerPort)[0],
                                "hostip" => $mapping->HostIp,
                                "hostport" => $mapping->HostPort,
                                "proto" => preg_split('/\//', $containerPort)[1]
                                )
                            );
                }
            }
        }
        $this->_bindMounts = array();
        $this->_timeSync = false;
        if (isset($containerData->HostConfig->Binds)) {
            foreach ($containerData->HostConfig->Binds as $bind) {
                if (strcmp($bind, "/etc/localtime:/etc/localtime:ro") === 0) {
                    $this->_timeSync = true;
                    continue;
                }
                $mode = "";
                if (isset(preg_split('/\:/', $bind)[2])) {
                    $mode = preg_split('/\:/', $bind)[2];
                }
                array_push(
                    $this->_bindMounts,
                    array(
                        "from" => preg_split('/\:/', $bind)[0],
                        "to" => preg_split('/\:/', $bind)[1],
                        "mode" => $mode
                    )
                );
            }
        }
        if (isset($containerData->Mounts)) {
            foreach ($containerData->Mounts as $mount) {
                if ((strcmp($mount->Mode, "") === 0) && (isset($mount->Driver)) && (strcmp($mount->Driver, "local") === 0)) {
                    array_push(
                        $this->_bindMounts,
                        array(
                            "from" => $mount->Destination,
                            "to" => "",
                            "mode" => $mount->Mode
                        )
                    );
                }
            }
        }
        $this->_names = ltrim($item->Names[0], "/");
        $this->_hasMounts = false;
        if (is_array($containerData->Mounts) && (count($containerData->Mounts) > 0)) {
            $this->_hasMounts = true;
        }
        $this->_volumesFrom = array();
        if (isset($containerData->HostConfig->VolumesFrom)) {
            foreach ($containerData->HostConfig->VolumesFrom as $volume) {
                array_push($this->_volumesFrom, array("from" => $volume));
            }
        }

        $this->_extraArgs = "";
        if (isset($containerData->Config->Labels->omv_docker_extra_args)) {
            $this->_extraArgs = $containerData->Config->Labels->omv_docker_extra_args;
        }

        $this->_containerCommand = "";
        if (isset($containerData->Config->Labels->omv_docker_container_command)) {
            $this->_containerCommand = $containerData->Config->Labels->omv_docker_container_command;
        }

        $this->_logPath = $containerData->LogPath;

        $this->_hostName = "";
        if (!(strcmp($containerData->Config->Hostname, "") === 0)) {
            $this->_hostName .= $containerData->Config->Hostname;
            if (!(strcmp($containerData->Config->Domainname, "") === 0)) {
                $this->_hostName .= "." . $containerData->Config->Domainname;
            }
        }
    }

    /**
     * Return id of the container
     *
     * @return string $_id
     * @access public
     */
    public function getId()
    {
        return substr($this->_id, 0, 12);
    }


    /**
     * Get the image the conatiner is mapped to
     *
     * @return string $_image
     * @access public
     */
    public function getImage()
    {
        return $this->_image;
    }

    /**
     * Get the status of the container
     *
     * @return string $_status
     * @access public
     */
    public function getStatus()
    {
        return $this->_status;
    }

    /**
     * Get the creation time of the conatiner
     *
     * @return string $_created
     * @access public
     */
    public function getCreated()
    {
        return $this->_created;
    }

    /**
     * Get the init command of the container
     *
     * @return string $_command
     * @access public
     */
    public function getCommand()
    {
        return $this->_command;
    }

    /**
     * Get the state of the container
     *
     * @return string $_state
     * @access public
     */
    public function getState()
    {
        return $this->_state;
    }

    /**
     * Get the port mappings of the container
     *
     * @return array $_ports
     * @access public
     */
    public function getPorts()
    {
        return $this->_ports;
    }

    /**
     * Get the network mode of the contanier
     *
     * @return string $_networkMode
     * @access public
     */
    public function getNetworkMode()
    {
        return $this->_networkMode;
    }

    /**
     * Get the privileged mode of the container
     *
     * @return bool $_privileged
     * @access public
     */
    public function getPrivileged()
    {
        return $this->_privileged;
    }

    /**
     * Get the restart policy of the container
     *
     * @return string $_restartPolicy
     * @access public
     */
    public function getRestartPolicy()
    {
        return $this->_restartPolicy;
    }

    /**
     * Get the max number of retries for on-failure policy
     *
     * @return string $_maxRetries
     * @access public
     */
    public function getMaxRetries()
    {
        return $this->_maxRetries;
    }

    /**
     * Get the environment variables defined in the container
     *
     * @return array $_envVars
     * @access public
     */
    public function getEnvironmentVariables()
    {
        return $this->_envVars;
    }

    /**
     * Get the image id of the image used to create the container
     *
     * @return string $_imageId
     * @access public
     */
    public function getImageId()
    {
        return $this->_imageId;
    }

    /**
     * Get the port bindings of the container
     *
     * @return array $_portBindings
     * @access public
     */
    public function getPortBindings()
    {
        return $this->_portBindings;
    }

    /**
     * Get the bind mounts in the container
     *
     * @return array $_bindMounts
     * @access public
     */
    public function getBindMounts()
    {
        return $this->_bindMounts;
    }

    /**
     * Get the name of the container
     *
     * @return string $_names
     * @access public
     */
    public function getName()
    {
        return $this->_names;
    }

    /**
     * Return true if the container has mountpoints
     *
     * @return bool $_hasMounts
     * @access public
     */
    public function hasMounts()
    {
        return $this->_hasMounts;
    }

    /**
     * Get the volumes from in the container
     *
     * @return array $_volumesFrom
     * @access public
     */
    public function getVolumesFrom()
    {
        return $this->_volumesFrom;
    }

    /**
     * Get the host name of the container
     *
     * @return string $_hostName
     * @access public
     */
    public function getHostName()
    {
        return $this->_hostName;
    }

    /**
     * Return true if the container is syncing time with the host
     *
     * @return bool $_syncTime
     * @access public
     */
    public function syncsTime()
    {
        return $this->_timeSync;
    }

    /**
     * Get the macvlan network name of the container
     *
     * @return string $_macVlanContainerNetwork
     * @access public
     */
    public function getMacVlanContainerNetwork()
    {
        return $this->_macVlanContainerNetwork;
    }

    /**
     * Get the macvlan network ip address of the container
     *
     * @return string $_macVlanContainerIpAddress
     * @access public
     */
    public function getMacVlanContainerIpAddress()
    {
        return $this->_macVlanContainerIpAddress;
    }

    /**
     * Get the value of the label property called omv_docker_extra_args use to stored the extra args
     *
     * @return string $_extraArgs
     * @access public
     */
    public function getExtraArgs()
    {
        return $this->_extraArgs;
    }

    /**
     * Get the value of the label property called omv_docker_container_command use to stored the container command
     *
     * @return string $_containerCommand
     * @access public
     */
    public function getContainerCommand()
    {
        return $this->_containerCommand;
    }

    /**
     * Get the value of the property LogPath from container info
     *
     * @return string $_logPath
     * @access public
     */
    public function getLogPath()
    {
        return $this->_logPath;
    }

}
