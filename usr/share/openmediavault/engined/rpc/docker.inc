<?php
/**
 * Copyright (c) 2015-2019 OpenMediaVault Plugin Developers
 *
 * @category OMVRpcServiceDocker
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

/*
require_once "openmediavault/object.inc";
require_once "openmediavault/config.inc";
require_once "openmediavault/error.inc";
require_once "openmediavault/util.inc";
require_once "openmediavault/rpcservice.inc";
require_once "openmediavault/notify.inc";
*/

use OMV\ErrorMsgException;
use OMV\Exception;
use OMV\Rpc\ServiceAbstract;
use OMV\Engine\Notify;
use OMV\Config\ConfigObject;
use OMV\Config\Database;
use OMV\Engine\Notify\Dispatcher;
use OMV\System\Process;
use OMV\Rpc\Rpc;

//use OMV\Rpc\ServiceManager;


require_once "/usr/share/omvdocker/Utils.php";
require_once "/usr/share/omvdocker/Image.php";

/**
 * RPC class for the Docker GUI plugin
 *
 * @category Class
 * @package  Openmediavault-docker-gui
 * @author   OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @license  http://www.gnu.org/copyleft/gpl.html GNU General Public License
 * @link     https://github.com/OpenMediaVault-Plugin-Developers/openmediavault-docker-gui
 *
 */
class OMVRpcServiceDocker extends ServiceAbstract
{

    private $dataModelPath = 'conf.service.docker';

    public function __construct(Database $database = null, Dispatcher $dispatcher = null){
        $this->database = $database ?: Database::getInstance();
        $this->dispatcher = $dispatcher ?: Dispatcher::getInstance();
    }

    /**
     * Get the name of the RPC service
     *
     * @return string
     */
    public function getName()
    {
        return "Docker";  // RPC Service name. Same as in .js files
    }

    /**
     * Get the main event message path of the service. This is a helper
     * function to avoid "magic numbers".
     *
     * @return string
     */
    private function _getEventMessagePath()
    {
        return "org.openmediavault.services.docker";
    }


    /**
     * Initialize the RPC service. Different methods of the RPC service are
     * declared here
     *
     * @return void
     */
    public function initialize()
    {
        $this->registerMethod("getImages");
        $this->registerMethod("pullImage");
        $this->registerMethod("deleteImage");
        $this->registerMethod("searchImages");
        $this->registerMethod("getImageData");

        $this->registerMethod("getContainers");
        $this->registerMethod("runContainer");
        $this->registerMethod("modifyContainer");
        $this->registerMethod("startContainer");
        $this->registerMethod("stopContainer");
        $this->registerMethod("restartContainer");
        $this->registerMethod("deleteContainer");
        $this->registerMethod("createContainer");
        $this->registerMethod("getVolumesFrom");
        $this->registerMethod("commitContainer");
        $this->registerMethod("modifyWarningEnabled");
        $this->registerMethod("refreshContainers");
        $this->registerMethod("getContainerData");

        $this->registerMethod("getNetworks");
        $this->registerMethod("createNetwork");
        $this->registerMethod("deleteNetwork");

        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
        $this->registerMethod("getDetails");
        $this->registerMethod("getLogs");
        $this->registerMethod("getRootFolders");
        $this->registerMethod("copyDockerData");
        $this->registerMethod("executeCommand");
        $this->registerMethod("syncDockerLogos");
        $this->registerMethod("getDockerRepo");

        $this->registerMethod("set");
        $this->registerMethod("getMacVlan");
        $this->registerMethod("clearLog");
        $this->registerMethod("connectNetwork");
        $this->registerMethod("disconnectNetwork");
        $this->registerMethod("getContainersNotInSelectedNetwork");
        $this->registerMethod("getContainersInSelectedNetwork");
    }

    public function set($params, $context)
    {
        // empty function for details window
        // TODO:  change window type for log and container modify
    }

    // Get system architecture
    private function getArch()
    {
        $uname = php_uname();
        if ( strpos($uname, "armv") !== false  ) {
            $arch = "armhf";
        } else {
            $arch = "amd64";
        }
        return $arch;
    }

    /**
     * Get all Docker images
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $objects An array with all docker macvlan network names and their respective subnet
     */
    public function getMacVlan($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $settings=$this->database->getAssoc($this->dataModelPath);

        $objects = [];
        $objects = OMVModuleDockerUtil::getMacVlanNetworks();
        return $objects;
    }


    /**
     * Get all Docker images
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $objects An array with all Docker images
     */
    public function getImages($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $settings=$this->database->getAssoc($this->dataModelPath);

        $objects = [];
        if ($settings['enabled']) {
            if ($settings["showDanglingImages"]) {
                $incDangling = false;
            } else {
                $incDangling = true;
            }

            $objects = OMVModuleDockerUtil::getImageList($incDangling);
        }
        return $objects;
    }

    /**
     * Pull a Docker image from the Docker Hub
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function pullImage($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.docker.pullimage");

        if (strcmp($params['tag'], "") === 0) {
            $params['tag'] = "latest";
        }

        // Create a background process.
        $bgStatusFilename = $this->createBgProcStatus();
        $pid = $this->fork();
        if ($pid > 0) {
            $this->initializeBgProcStatus($bgStatusFilename, $pid);
            return $bgStatusFilename;
        }

        // Child process.
        try {
            $bgOutputFilename = $this->createBgProcOutput();
            $this->updateBgProcStatus(
                $bgStatusFilename,
                "outputfilename",
                $bgOutputFilename
            );
            $cmd = "docker pull \"" . $params['repository'] . ":" .
                $params['tag'] . "\"";
            $this->exec($cmd, $output, $bgOutputFilename);
            $this->finalizeBgProcStatus($bgStatusFilename, $output);

        } catch(Exception $e) {
            $this->finalizeBgProcStatus($bgStatusFilename, "", $e);
            exit(1);
        }
    }

    /**
     * Deletes a Docker image
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function deleteImage($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.docker.deleteid");
        $cmd = "docker rmi " . $params['id'];
        $process = new Process($cmd);
        $process->execute();
    }

    /**
     * Searches the Docker Hub for a specific repository
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $objects An array ith all repositories found
     */
    public function searchImages($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $objects = [];
        $settings=$this->database->getAssoc($this->dataModelPath);

        $url = "http::/images/search?term=" . $params["name"];
        $response = OMVModuleDockerUtil::doApiCall($url);
        $data = json_decode($response);

        foreach ($data as $repo) {
            switch ($params['filterParam']) {
            case "all":
                array_push(
                    $objects,
                    [
                        "name" => $repo->name,
                        "description" => $repo->description,
                        "stars" => $repo->star_count
                    ]
                );
                break;
            case "trusted":
                if ($repo->is_trusted) {
                    array_push(
                        $objects,
                        [
                            "name" => $repo->name,
                            "description" => $repo->description,
                            "stars" => $repo->star_count
                        ]
                    );
                }
                break;
            case "official":
                if ($repo->is_official) {
                    array_push(
                        $objects,
                        [
                            "name" => $repo->name,
                            "description" => $repo->description,
                            "stars" => $repo->star_count
                        ]
                    );
                }
                break;
            case "trustedofficial":
                if ($repo->is_official && $repo->is_trusted) {
                    array_push(
                        $objects,
                        [
                            "name" => $repo->name,
                            "description" => $repo->description,
                            "stars" => $repo->star_count
                        ]
                    );
                }
                break;
            default:
                array_push(
                    $objects,
                    [
                        "name" => $repo->name,
                        "description" => $repo->description,
                        "stars" => $repo->star_count
                    ]
                );
                break;
            }

        }
        return $objects;
    }

    /**
     * Get data of specific Image
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $objects An array ith all repositories found
     */
    public function getImageData($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);

        $settings=$this->database->getAssoc($this->dataModelPath);
        $image = OMVModuleDockerUtil::getImage($params['id']);

        $object = [
            "repository" => rtrim(ltrim($image->getRepository(), "<"), ">"),
            "tag" => rtrim(ltrim($image->getTag(), "<"), ">"),
            "id" => $image->getId(),
            "created" => $image->getCreated(),
            "size" => $image->getSize(),
            "ports" => $image->getPorts(),
            "envvars" => $image->getEnvVars(),
            "imagevolumes" => $image->getVolumes()
        ];
        return $object;
    }

    /**
     * Get all Docker containers
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $objects An array with all containers
     */
    public function getContainers($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);

        $objects = [];
        $settings=$this->database->getAssoc($this->dataModelPath);

        // Determine if the user is using a non-default docker datarootpath
        if (!empty($settings['sharedfolderref'])) {
            $datarootpath = Rpc::call(
                "ShareMgmt",
                "getPath",
                [
                    "uuid" => $settings['sharedfolderref']
                ],
                $context
            );
        } else {
            $datarootpath = "/var/lib/docker";
        }

        if ($settings['enabled']) {
            $objects = OMVModuleDockerUtil::getContainerList($datarootpath);
        }
        return $objects;
    }

    /**
     * Run a single Docker container
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function runContainer($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);

        $cmd = "docker run -d ";
        switch ($params['restartpolicy']) {
            case "always":
                $cmd .= "--restart=always ";
                break;
            case "no":
                $cmd .= "--restart=no ";
                break;
            case "unless-stopped":
                $cmd .= "--restart=unless-stopped ";
                break;
            case "on-failure":
                if (isset($params['maxretries'])) {
                    $cmd .= "--restart=on-failure:" . $params['maxretries'] . " ";
                } elseif (!isset($params['maxretries'])) {
                    $cmd .= "--restart=on-failure ";
                }
                break;
        }

        //Check if privileged checkbox is enabled
        if ($params['privileged']) {
            $cmd .= "--privileged ";
        }

        //Check if time sync checkbox is enabled
        if ($params['timeSync']) {
            $cmd .= "-v /etc/localtime:/etc/localtime:ro ";
        }

        //Check which networking mode is selected
        switch ($params['networkMode']) {
        case "Bridge":
            $cmd .= "--net=bridge ";
            foreach ($params['portForwards'] as $portForward) {
                if (!(strcmp($portForward['customPort'], "") === 0)) {
                    $destport = $portForward['customPort'] . "/" . $portForward['proto'];
                } else {
                    $destport = $portForward['exposedPort'];
                }
                if (!(strcmp($portForward['hostip'], "") === 0)) {
                    $cmd .= "-p " . $portForward['hostip'] . ":" .
                        $portForward['hostport'] . ":" . $destport . " ";
                }
            }
            //Check if host name is specified
            if ((!empty($params['hostName']))) {
                $cmd .= "-h \"" . $params['hostName'] . "\" ";
            }
            break;
        case "Host":
            $cmd .= "--net=host ";
            break;
        case "None":
            $cmd .= "--net=none ";
            break;
        case "Macvlan":
            $cmd .= "--net=" . $params['macvlan_network'] . " " . "--ip=" . $params['macvlan_ipaddress'] . " ";
            if ((!empty($params['hostName']))) {
                $cmd .= "-h \"" . $params['hostName'] . "\" ";
            }
            break;
        }

        //Check for environment variables
        foreach ($params['envVars'] as $envVar) {
            if (!(strcmp($envVar['name'], "") === 0)) {
                $cmd .= "-e " . $envVar['name'] . "=\"" . $envVar['value'] . "\" ";
            }
        }

        //Check for bind mounts
        //Container path must not be set to "/"
        //If Container path is empty a new volume is created instead
        //of bindmount
        foreach ($params['bindMounts'] as $bindMount) {
            if (strcmp($bindMount['to'], "/") === 0) {
                throw new OMVModuleDockerException(
                    "Container path must not be set to \"/\""
                );
            }
            if (!(strcmp($bindMount['from'], "") === 0)) {
                if (!(strcmp($bindMount['to'], "") === 0)) {
                    $cmd .= "-v \"" . $bindMount['from'] . "\":\"" .
                        $bindMount['to'] . "\"";
                    if ($bindMount['romode']) {
                        $cmd .= ":ro ";
                    } else {
                        $cmd .= ":rw ";
                    }
                } else {
                    $cmd .= "-v \"" . $bindMount['from'] . "\" ";
                }
            }
        }

        //Check for volumes from
        foreach ($params['volumes'] as $volume) {
            if (!(strcmp($volume['from'], "") === 0)) {
                $cmd .= "--volumes-from \"" . $volume['from'] . "\" ";
            }
        }

        //Check if a custom container name has been supplied
        if (!(strcmp($params['containerName'], "") === 0)) {
            $cmd .= "--name=\"" . $params['containerName'] . "\" ";
        }

        //Check if extra arguments are supplied
        $extraArgs = trim($params['extraArgs']);
        if (!(strcmp($extraArgs, "") === 0)) {
            //Store the extra command parameters in a container label as omv_docker_extra_args
            $cmd .= "--label omv_docker_extra_args=" . "\"" . $extraArgs . "\" ";
            $cmd .= $extraArgs . " ";
        }

        $containerCommand = trim($params['containerCommand']);
        if(!(strcmp($containerCommand, "") === 0)) {
          //Store the container command in a container label as omv_docker_container_command
          $cmd .= "--label omv_docker_container_command=" . "\"" . $containerCommand . "\" ";
        }

        //Add image to command
        $cmd .= "\"" . $params['image'] . "\" ";

        if(!(strcmp($containerCommand, "") === 0)) {
          //Add the container command to command
          $cmd .= $containerCommand . " ";
        }

        //throw new OMVModuleDockerException($cmd);
        $process = new Process($cmd);
        $process->setRedirect2to1();
        $process->execute();

    }

    /**
     * Modify a single Docker container
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function modifyContainer($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $settings=$this->database->getAssoc($this->dataModelPath);

        $container = OMVModuleDockerUtil::getContainer($params['cid']);
        $image = OMVModuleDockerUtil::getImage(
            substr($container->getImageId(), 0, 12));

        $restartpolicy = $container->getRestartPolicy();
        $maxretries = $container->getMaxRetries();
        $networkMode = $container->getNetworkMode();
        $macvlan_network = $container->getMacVlanContainerNetwork();
        $macvlan_ipaddress = $container->getMacVlanContainerIpAddress();
        $portForwards = [];
        foreach ($container->getPortBindings() as $binding) {
            $customPort = "";
            $exposedPort = "";
            foreach ($image->getPorts() as $port) {
                if (strcmp($binding['containerportstring'], $port['name']) === 0) {
                    $exposedPort = explode("/", $port['name'])[0];
                    break;
                }
            }
            if (strcmp($exposedPort, "") === 0) {
                $customPort = explode("/", $binding['containerportstring'])[0];
            }
            array_push($portForwards,
                [
                    "customPort" => $customPort,
                    "exposedPort" => $exposedPort,
                    "hostip" => $binding['hostip'],
                    "hostport" => $binding['hostport'],
                    "proto" => $binding['proto']
                ]
            );

        }
        $envVars = [];
        foreach ($container->getEnvironmentVariables() as $key => $value) {
            array_push($envVars,
                [
                    "name" => $key,
                    "value" => $value
                ]
            );
        }
        $bindMounts = [];
        foreach($container->getBindMounts() as $mount) {
            array_push($bindMounts,
                [
                    "from" => $mount['from'],
                    "to" => $mount['to'],
                    "romode" => $mount['mode']
                ]
            );
        }

        $old_params = [
            "restartpolicy" => $restartpolicy,
            "maxretries" => $maxretries,
            "privileged" => $container->getPrivileged(),
            "timeSync" => $container->syncsTime(),
            "networkMode" => ucfirst($container->getNetworkMode()),
            "macvlan_network" => $container->getMacVlanContainerNetwork(),
            "macvlan_ipaddress" => $container->getMacVlanContainerIpAddress(),
            "portForwards" => $portForwards,
            "hostName" => $container->getHostName(),
            "envVars" => $envVars,
            "bindMounts" => $bindMounts,
            "volumes" => $container->getVolumesFrom(),
            "extraargs" => $container->getExtraArgs(),
            "containerName" => $container->getName(),
            "image" => $container->getImage(),
            "containerCommand" => $container->getContainerCommand()
        ];

        //Remove existing container
        $cmd = "docker stop " . $params['cid'] . " && docker rm " . $params['cid'];
        $process = new Process($cmd);
        $process->execute();

        if ($params['refresh']) {
            $params = $old_params;
        }

        //Try to start up a new container with the given parameters.
        //Revert to old parameters upon failure
        try {
            $this->runContainer($params, $context);
        } catch (OMVModuleDockerException $e) {
            $this->runContainer($old_params, $context);
            $e_str = explode("'", (string)$e);
            return (["response" => $e_str[3]]);
        }
    }

    /**
     * Start one or multiple Docker containers
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function startContainer($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $cmd = "docker start " . $params['id'];
        $process = new Process($cmd);
        $process->setRedirect2to1();
        $process->execute();
    }

    /**
     * Stop one or multiple Docker containers
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function stopContainer($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $cmd = "docker stop " . $params['id'];
        $process = new Process($cmd);
        $process->setRedirect2to1();
        $process->execute();
    }

    /**
     * Restart one or multiple Docker containers
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function restartContainer($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $cmd = "docker restart " . $params['id'];
        $process = new Process($cmd);
        $process->setRedirect2to1();
        $process->execute();
    }

    /**
     * Delete one or multiple Docker containers
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function deleteContainer($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.docker.deleteid");
        $cmd = "docker rm " . $params['id'];
        $process = new Process($cmd);
        $process->setRedirect2to1();
        $process->execute();
    }

    /**
     * Clear the log on one or multiple containers
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function clearLog($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $settings=$this->database->getAssoc($this->dataModelPath);
        $ids = explode(" ", $params['id']);
        foreach ($ids as $id) {
            $container = OMVModuleDockerUtil::getContainer($id);
            $logpath = $container->getLogPath();
            $cmd = "cat /dev/null >" . $logpath;
            $process = new Process($cmd);
            $process->execute();
        }
    }

    /**
     * Create a new Docker container
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function createContainer($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $cmd = "docker create ";

        //Check for bind mounts
        //Container path must not be set to "/"
        //If Container path is empty a new volume is created instead
        //of bindmount
        foreach ($params['bindMounts'] as $bindMount) {
            if (strcmp($bindMount['to'], "/") === 0) {
                throw new OMVModuleDockerException(
                    "Container path must not be set to \"/\""
                );
            }
            if (!(strcmp($bindMount['from'], "") === 0)) {
                if (!(strcmp($bindMount['to'], "") === 0)) {
                    $cmd .= "-v \"" . $bindMount['from'] . "\":\"" .
                        $bindMount['to'] . "\" ";
                } else {
                    $cmd .= "-v \"" . $bindMount['from'] . "\" ";
                }
            }
        }

        //Check if a custom container name has been supplied
        if (!(strcmp($params['containerName'], "") === 0)) {
            $cmd .= "--name=" . $params['containerName'] . " ";
        }

        $cmd .= "\"" . $params['imageRepo'] . "\" /bin/true";
        $process = new Process($cmd);
        $process->execute();
    }

    /**
     * Get all containers with volumes mounted
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $objects An array with all containers
     */
    public function getVolumesFrom($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $objects = [];
        $settings=$this->database->getAssoc($this->dataModelPath);

        if ($settings['enabled']) {
            $containers = OMVModuleDockerUtil::getContainers();
            foreach ($containers as $container) {
                if ($container['hasmounts']) {
                    array_push($objects, ["name" => $container['name']]);
                }
            }
        }
        return $objects;
    }

    /**
     * Commit a container to a new image
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function commitContainer($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $cmd = "docker commit " . $params['name'] . " " . $params['newImage'];
        $process = new Process($cmd);
        $process->setRedirect2to1();
        $process->execute();
    }

    /**
     * Check if a warning should be displayed when modifying a container
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function modifyWarningEnabled($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $settings=$this->database->getAssoc($this->dataModelPath);
        if ($settings["cwarn"]) {
            return ["response" => "true"];
        } else {
            return null;
        }
    }

    /**
     * Refresh all containers based on a specific image
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function refreshContainers($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $settings=$this->database->getAssoc($this->dataModelPath);
        $image = $params['image'];
        $containers = OMVModuleDockerUtil::getContainers();

        foreach ($containers as $c) {
            if (strcmp($c['image'], $image) === 0) {
                $new_params = [
                    "cid" => $c['id'],
                    "refresh" => true
                ];
                $this->modifyContainer($new_params, $context);
            }
        }
    }

    /**
     * Get data of a single container
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function getContainerData($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $settings=$this->database->getAssoc($this->dataModelPath);
        $container = OMVModuleDockerUtil::getContainer($params['id']);
        $ports = "";
        foreach ($container->getPorts() as $exposedport => $hostports) {
            if ($hostports) {
                foreach ($hostports as $hostport) {
                    $ports .= $hostport["HostIp"] . ":" .
                        $hostport["HostPort"] .
                        "->" . $exposedport . ", ";
                }
            } else {
                $ports .= $exposedport . ", ";
            }
        }
        $ports = rtrim($ports, ", ");
        $image = OMVModuleDockerUtil::getImage(
            substr($container->getImageId(), 0, 12));

        $object = [
            "id" => $container->getId(),
            "image" => $container->getImage(),
            "command" => $container->getCommand(),
            "created" => $container->getCreated(),
            "state" => $container->getState(),
            "status" => $container->getStatus(),
            "name" => $container->getName(),
            "privileged" => $container->getPrivileged(),
            "restartpolicy" => $container->getRestartPolicy(),
            "maxretries" => $container->getMaxRetries(),
            "networkmode" => ucfirst($container->getNetworkMode()),
            "macvlan_network" => $container->getMacVlanContainerNetwork(),
            "macvlan_ipaddress" => $container->getMacVlanContainerIpAddress(),
            "envvars" => $image->getEnvVars(),
            "cenvvars" => $container->getEnvironmentVariables(),
            "exposedports" => $image->getPorts(),
            "portbindings" => $container->getPortBindings(),
            "bindmounts" => $container->getBindMounts(),
            "ports" => $ports,
            "hasmounts" => $container->hasMounts(),
            "volumesfrom" => $container->getVolumesFrom(),
            "extraargs" => $container->getExtraArgs(),
            "hostname" => $container->getHostName(),
            "timesync" => $container->syncsTime(),
            "imagevolumes" => $image->getVolumes(),
            "containercommand" => $container->getContainerCommand()
        ];
        return $object;
    }

    /**
     * Get all Docker networks
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $objects An array with all Docker networks
     */
    public function getNetworks($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $settings=$this->database->getAssoc($this->dataModelPath);

        $objects = [];
        if ($settings['enabled']) {
            $objects = OMVModuleDockerUtil::getNetworkList();
        }
        return $objects;
    }

    /**
     * Create a new Docker network
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function createNetwork($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $cmd = "docker network create " .
        "-d {$params['driver']} --subnet={$params['subnet']} ".
        "--gateway={$params['gateway']} -o parent={$params['parent']} " .
        "{$params['name']}";

        $process = new Process($cmd);
        $process->execute();
    }

    /**
     * Deletes a Docker network
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function deleteNetwork($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.docker.deletename");
        $cmd = "docker network rm {$params['name']}";
        $process = new Process($cmd);
        $process->execute();
    }


    /**
     * Retrieve all plugin settings
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An ssociative array with all settings
     */
    public function getSettings($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);

        $dockerVersion = "0";
        $dockerVersionTxt = "";
        $cmd = "docker --version";
        $process = new Process($cmd);
        $process->setRedirect2to1();
        $out = $process->execute();

        if (!preg_match('/^Docker version.*$/', $out, $matches)) {
            $dockerVersionTxt = "Docker not installed";
            $dockerInfoTxt = "Docker not installed";
        } else {
            $dockerVersion = $matches[1];
            $cmd = "docker version";
            unset($out);
            $process = new Process($cmd);
            $process->setQuiet();
            $process->execute($out);
            $dockerVersionTxt = implode("\n", $out);

            unset($cmd,$out);
            $cmd = "docker info";
            $process = new Process($cmd);
            $process->setQuiet();
            $process->execute($out);
            $dockerInfoTxt = implode("\n", $out);

        }

        $settings=$this->database->getAssoc($this->dataModelPath);

        //$object["showDanglingImages"] = boolval(
        //$object["showDanglingImages"]);
        $settings["enabled"] = boolval($settings["enabled"]);
        //To be changed to docker info
        $settings["dockerInfo"] = $dockerInfoTxt;
        $settings["dockerVersion"] = $dockerVersionTxt;
        $settings["sharedfolderref"] = $settings["sharedfolderref"];
        $settings["cwarn"] = boolval($settings["cwarn"]);

        return $settings;
    }

    /**
     * Set all plugin settings
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An associative array with all the new settings
     */

    public function setSettings($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.docker.setsettings");

        //Get the old settings object
        $oldSettings = $this->database->getAssoc($this->dataModelPath);
        $oldsharedFolder = $oldSettings['sharedfolderref'];
        $sharedFolder = $params['sharedfolderref'];

//      This will check that if the plugin is enabled and the shared folder reference is different from the current one, will regenerate /etc/docker/daemon.json
//      and restart the dockerd. This will prevent restart of the dockerd if the user changes the container modification warning setting.
        if ($params['enabled'] && !(strcmp($params['sharedfolderref'], $oldsharedFolder) === 0)) {
            OMVModuleDockerUtil::changeDockerSettings($context, $params);
        }

        // Update the configuration object.
        $object = [
            "enabled" => array_boolval($params, "enabled"),
            "sharedfolderref" => $params['sharedfolderref'],
            "cwarn" => array_boolval($params, "cwarn")
        ];

        $config = new ConfigObject($this->dataModelPath);
        $config->setAssoc($object);
        $this->database->set($config);

        $this->dispatcher->notify(
            OMV_NOTIFY_MODIFY,
            $this->_getEventMessagePath(),
            $object
        );

        return $object;
    }

    /**
     * Get detailed info on a Docker image or container
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return string The details info.
     */
    public function getDetails($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $cmd = "docker inspect " . $params['id'];
        $process = new Process($cmd);
        $process->setRedirect2to1();
        $process->execute($out);
        return implode("\n", $out);
    }

    /**
     * Get logs from a Docker container
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An associative array with all the new settings
     */
    public function getLogs($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $cmd = "docker logs " . $params['id'];
        $process = new Process($cmd);
        $process->setRedirect2to1();
        $process->execute($out);
        return ["logs" => implode("\n\r", $out)];
    }

    /**
     * Get folder structure for folder browser
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $result An array with folders
     */
    public function getRootFolders($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);

        // List the directory contents.
        $result = [];
        $path = sprintf("/%s", rtrim($params['path'], "/"));
        if (strlen($path) == 0) {
            $path = "/";
        }

        foreach (new DirectoryIterator($path) as $item) {
            if ($item->isDot()) {
                continue;
            }
            if (!$item->isDir()) {
                continue;
            }
            $result[] = $item->getFilename();
        }
        return $result;
    }

    /**
     * Copy Docker data from one folder to another
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function copyDockerData($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.docker.copydockerdata");

        $absPath = "";
        if ($params['sharedfolderref']) {
            $share = Rpc::call("ShareMgmt","get", ["uuid" => $params['sharedfolderref']], $context);
            $mntent = Rpc::call("FsTab","get", ["uuid" => $share['mntentref']], $context);

            $absPath = rtrim(
                rtrim(
                    $mntent['dir'], "/"
                )
                . "/" . $share['reldirpath'], "/"
            );
        }
        if ((strcmp($params['srcpath'], "") === 0) || (strcmp($absPath, "") === 0)) {
            throw new OMVModuleDockerException(
                "Either source or destination " .
                "path is empty"
            );
        }

        $srcpath = $params['srcpath'];
        $destpath = $absPath;
        OMVModuleDockerUtil::stopDockerService();

        if (file_exists($srcpath . "/containers") && file_exists($srcpath . "/volumes")) {
            if (file_exists($srcpath . "/devicemapper/mnt")) {
                $cmd = "umount $srcpath/devicemapper/mnt/*";
                $process = new Process($cmd);
                $process->setQuiet();
                $process->execute();
            }
            // Create a background process.
            $bgStatusFilename = $this->createBgProcStatus();
            $pid = $this->fork();
            if ($pid > 0) {
                $this->initializeBgProcStatus($bgStatusFilename, $pid);
                return $bgStatusFilename;
            }

            // Child process.
            try {
                $bgOutputFilename = $this->createBgProcOutput();
                $this->updateBgProcStatus(
                    $bgStatusFilename,
                    "outputfilename",
                    $bgOutputFilename
                );
                $cmd = "cp -R $srcpath/* $destpath";
                $this->exec($cmd, $output, $bgOutputFilename);
                $this->finalizeBgProcStatus($bgStatusFilename, $output);

            } catch(Exception $e) {
                $this->finalizeBgProcStatus($bgStatusFilename, "", $e);
                exit(1);
            }
        } else {
            OMVModuleDockerUtil::startDockerService();
            throw new OMVModuleDockerException(
                "Not a valid Docker source path specified"
            );
        }
        OMVModuleDockerUtil::startDockerService();
    }

    /**
     * Execute command inside running Docker container
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return void
     */
    public function executeCommand($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.docker.executecommand");

        // Create a background process.
        $bgStatusFilename = $this->createBgProcStatus();
        $pid = $this->fork();
        if ($pid > 0) {
            $this->initializeBgProcStatus($bgStatusFilename, $pid);
            return $bgStatusFilename;
        }

        // Child process.
        try {
            $bgOutputFilename = $this->createBgProcOutput();
            $this->updateBgProcStatus(
                $bgStatusFilename,
                "outputfilename",
                $bgOutputFilename
            );
            $cmd = "docker exec -t " . $params['containerId'] . " " .
                $params['cmd'];
            $this->exec($cmd, $output, $bgOutputFilename);
            $this->finalizeBgProcStatus($bgStatusFilename, $output);

        } catch(Exception $e) {
            $this->finalizeBgProcStatus($bgStatusFilename, "", $e);
            exit(1);
        }
    }

    /**
     * Retrieve Docker logos from various sources
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $objects An associative array with all repos
     */
    public function syncDockerLogos($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $logoPath = "/var/www/openmediavault/images/dockerrepo/";
        $cmd = "mkdir -p $logoPath";
        $process = new Process($cmd);
        $process->execute();

        //First populate with logos from OMV repo
        $baseUrl = "http://openmediavault-plugin-developers.github.io/" .
            "openmediavault-docker-repo/";
        $imgBase = $baseUrl . "logos/";
        $repoUrl = sprintf("%sdockerlist_%s.xml", $baseUrl, $this->getArch());
        $data = file_get_contents($repoUrl);
        $xml = simplexml_load_string($data);
        foreach ($xml as $image) {
            //Use blank logo if no imageurl is specified
            if (strlen($image->imageurl) !== 0) {
                copy($imgBase . $image->imageurl, $logoPath . $image->imageurl);
            }
        }

        //Next populate with linuxserver.io logos
        $url = "http://tools.linuxserver.io/docker-templates.json";
        $local_logo_path = "/var/www/openmediavault/images/dockerrepo/";
        $json = file_get_contents($url);
        $data = json_decode($json);
        foreach ($data->applist as $image) {
            $imgRepo = trim((string)$image->Repository);
            if (strlen($image->Icon) !== 0) {
                $logo_url = trim((string)$image->Icon);
                $repo_name = trim((string)$image->Repository);
                preg_match('/^.*(\.[^\.]+)$/', $logo_url, $matches);
                $logo_suffix = $matches[1];
                $logo_name = str_replace("/", "_", $repo_name) . $logo_suffix;
                $new_hash = trim((string)$image->Icon_SHA);
                $local_hash_path = $local_logo_path . str_replace("/", "_", $repo_name) . ".sha";
                $old_hash = "";
                if (file_exists($local_hash_path)) {
                    $old_hash = trim(file_get_contents($local_hash_path));
                }
                if (strcmp($new_hash, $old_hash) !== 0) {
                    try {
                        $imagick = new Imagick($logo_url);
                        $imagick->scaleImage(40, 40, true);
                        $imagick->modulateImage(100, 0, 100);
                        $imagick->writeImage($local_logo_path . $logo_name);
                        $imagick->clear();
                        file_put_contents($local_hash_path, $new_hash);
                    } catch (ImagickException $e) {
                        //Do nothing
                    }
                }
            }
        }
    }

    /**
     * Retrieve list of Docker repos from Github source
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $objects An associative array with all repos
     */
    public function getDockerRepo($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $settings = $this->database->getAssoc($this->dataModelPath);

        $pulledImages = [];
        $cmd = "docker images";
        $process = new Process($cmd);
        $process->setRedirect2to1();
        $process->execute($out);
        for ($i = 1; $i < count($out); $i++) {
            preg_match('/^([\S]+)\s+\S+\s+([\S]+).*$/', $out[$i], $matches);
            $imgName = $matches[1];
            $pulledImages[trim($imgName)] = $matches[2];
        }
        $logoPath = "/var/www/openmediavault/images/dockerrepo/";
        $cmd = "mkdir -p $logoPath";
        $process = new Process($cmd);
        $process->execute();
        try{
            //First populate with images from OMV repo
            $baseUrl = "http://openmediavault-plugin-developers.github.io/" .
                "openmediavault-docker-repo/";
            $imgBase = $baseUrl . "logos/";
            $repoUrl = sprintf("%sdockerlist_%s.xml", $baseUrl, $this->getArch());
            $data = file_get_contents($repoUrl);
            $xml = simplexml_load_string($data);
            $objects = [];
            foreach ($xml as $image) {
                //Use blank logo if no logo is found
                $logo = "";
                if (file_exists($logoPath . $image->imageurl)) {
                    $logo = $image->imageurl;
                }
                //Silently skip any records that are missing data
                if ((strlen($image->name) === 0) || (strlen($image->desc) === 0) || (strlen($image->category) === 0) || (strlen($image->repo) === 0)) {
                    continue;
                } else {
                    $ports_ary = [];
                    $pulled = false;
                    $imgRepo = trim((string)$image->repo);
                    if (isset($pulledImages[$imgRepo])) {
                        $pulled = true;
                        $image_obj = OMVModuleDockerUtil::getImage(
                            $pulledImages[$imgRepo]
                        );
                        $ports_ary = $image_obj->getPorts();
                    }
                    $temp = ["name" => (string)$image->name,
                        "category" => (string)$image->category,
                        "logo" => (string)$logo,
                        "desc" => (string)$image->desc,
                        "repo" => (string)$image->repo,
                        "pulled" => $pulled,
                        "timesync" => true,
                        "restartpolicy" => "always",
                        "privileged" => false,
                        "cenvvars" => [],
                        "envvars" => [],
                        "ports" => $ports_ary,
                        "portbindings" => [],
                        "extraargs" => "",
                        "isupdated" => false
                    ];
                    array_push($objects, $temp);
                }
            }

            //Next populate with linuxserver.io images
            $url = "http://tools.linuxserver.io/docker-templates.json";
            $local_logo_path = "/var/www/openmediavault/images/dockerrepo/";
            $json = file_get_contents($url);
            $data = json_decode($json);
            foreach ($data->applist as $image) {

                $cenv_ary = [];
                $ports_ary = [];
                $pulled = false;
                $updated = false;
                $imgRepo = trim((string)$image->Repository);
                if (isset($pulledImages[$imgRepo])) {
                    $pulled = true;
                    $image_obj = OMVModuleDockerUtil::getImage(
                        $pulledImages[$imgRepo]
                    );
                    $ports_ary = $image_obj->getPorts();
                    $local_time = (int)date("U", $image_obj->getTimestamp());
                    $repo_time = (int)date("U", trim((string)$image->Updated));
                    if ($local_time < $repo_time) {
                        $updated = true;
                    }
                }
                $timesync = true;
                if (strcmp(trim((string)$image->BindTime), "false") === 0) {
                    $timesync = false;
                }
                $privileged = false;
                if (strcmp(trim((string)$image->Privileged), "true") === 0) {
                    $privileged = true;
                }
                foreach ($image->Environment->Variable as $cenvvar) {
                    if ((strcmp(trim((string)$cenvvar->Name), "PUID") === 0) ||
                        (strcmp(trim((string)$cenvvar->Name), "PGID") === 0)) {
                        continue;
                    }
                    $cenv_ary[$cenvvar->Name] = $cenvvar->Value;
                }
                $cenv_ary["PUID"] = 65534;
                $cenv_ary["PGID"] = 65534;
                $nw_mode = ucfirst(trim((string)$image->Networking->Mode));
                $port_bindings = [];
                if (strcmp($nw_mode, "Bridge") === 0) {
                    foreach ($image->Networking->Publish[0]->Port as $port) {
                        array_push(
                            $port_bindings,
                            [
                                "containerportstring" => trim((string)$port->ContainerPort) . "/" . trim((string)$port->Protocol),
                                "containerportnr" => trim((string)$port->ContainerPort),
                                "hostip" => "0.0.0.0",
                                "hostport" => trim((string)$port->HostPort),
                                "proto" => "Select"
                            ]);
                    }
                }
                $extraargs = "";
                if ((isset($image->ExtraParams)) && is_array($image->ExtraParams)) {
                    foreach ($image->ExtraParams as $param) {
                        $extraargs .= trim((string)$param) . " ";
                    }
                }
                $extraargs = trim($extraargs);
                $categories = explode(":", trim((string)$image->Category));
                $categories = implode("<br>", $categories);

                //Use blank logo if no Icon is specified
                $logo = "";
                if (strcmp(trim((string)$image->Icon), "") !== 0) {
                    $logo_url = trim((string)$image->Icon);
                    $repo_name = trim((string)$image->Repository);
                    preg_match('/^.*(\.[^\.]+)$/', $logo_url, $matches);
                    $logo_suffix = $matches[1];
                    $logo_name = str_replace("/", "_", $repo_name) . $logo_suffix;
                    if (file_exists($local_logo_path . $logo_name)) {
                        $logo = $logo_name;
                    }
                }
                $desc = "No description available";
                if (isset($image->Overview)) {
                    $desc = trim((string)$image->Overview);
                }

                $temp = ["name" => trim((string)$image->Name),
                    "category" => $categories,
                    "logo" => $logo,
                    "desc" => $desc,
                    "repo" => trim((string)$image->Repository),
                    "pulled" => $pulled,
                    "timesync" => $timesync,
                    "restartpolicy" => "always",
                    "privileged" => $privileged,
                    "cenvvars" => $cenv_ary,
                    "envvars" => [],
                    "ports" => $ports_ary,
                    "networkmode" => $nw_mode,
                    "portbindings" => $port_bindings,
                    "extraargs" => $extraargs,
                    "isupdated" => $updated
                ];
                array_push($objects, $temp);
        }
        }catch (Exception $e){
            if(!isset($objects))
                $objects = [];
        }finally{
            return $objects;
        }
    }

    //throw new OMVModuleDockerException($cmd);

    /**
     * Connect the selected container to a specific docker network
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An associative array with all the new settings
     */
    public function connectNetwork($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        if (empty($params['ipaddress'])) {
            $cmd = "docker network connect " . $params['networkname'] . " " . $params['containername'];
        } elseif (isset($params['ipaddress'])) {
            $cmd = "docker network connect " . "--ip " . $params['ipaddress'] . " " . $params['networkname'] . " " . $params['containername'];
        }
        $process = new Process($cmd);
        $process->setRedirect2to1();
        $process->execute($out);
        return ["details" => implode("\n\r", $out)];
    }

    /**
     * Disconnect the selected container to a specific docker network
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An associative array with all the new settings
     */
    public function disconnectNetwork($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        foreach ($params['containername'] as $container) {
            $cmd = "docker network disconnect " . $params['networkname'] . " " . $container;
            $process = new Process($cmd);
            $process->setRedirect2to1();
            $process->execute($out);
            return ["details" => implode("\n\r", $out)];
        }
    }



    /**
     * Get all the containers that are not connected to the current selected network
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An associative array with all the containers name
     */
    public function getContainersNotInSelectedNetwork($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);

        $objects = [];
        $settings=$this->database->getAssoc($this->dataModelPath);
        if ($settings['enabled']) {
            $objects = OMVModuleDockerUtil::getContainersNotInSelectedNetworkList($params['selectednetwork']);
        }
        return $objects;
    }

    /**
     * Get all the containers that are connected to the current selected network
     *
     * @param array  $params  An associative array with all RPC call parameters
     * @param string $context The context of the user maing the RPC call
     *
     * @return array $object An associative array with all the containers name
     */
    public function getContainersInSelectedNetwork($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);

        $objects = [];
        $settings=$this->database->getAssoc($this->dataModelPath);
        if ($settings['enabled']) {
            $objects = OMVModuleDockerUtil::getContainersInSelectedNetworkList($params['selectednetwork']);
        }
        return $objects;
    }

}
