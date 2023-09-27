<?php

$socket_path = "/tmp/php-c-socket";
$data = "Hello from PHP!";

$socket = socket_create(AF_UNIX, SOCK_STREAM, 0);

socket_connect($socket, $socket_path);

socket_write($socket, $data, strlen($data));

socket_close($socket);