
<?php
/*
class BDD_connector {
	var $name;
	var $host;
	var $port;
	var $user;
	var $pass;
	
	function BDD_connector($nameDB, $hostDB, $portDB, $userDB, $passDB) {

		$this->name = $nameDB;
		$this->host = $hostDB;
		$this->port = $portDB;
		$this->user = $userDB;
		$this->pass = $passDB;
		
		if (!$DBquery = pg_connect($this->name, $this->host, $this->port, $this->user, $this->pass)) {
			echo 'Connexion impossible à tagraisse';
			exit;
		}
		
	}
}

$base = "postgres";
$serveur = "localhost";
$port = "5432";
$login = "postgres";
$passe = "toor";

$connect = new BDD_connector($base, $serveur, $port, $login, $passe);


*/
$conn_string = "host=localhost port=5432 user=postgres password=toor";
$dbconn = pg_connect($conn_string) or die ("Could not connect to server\n"); 

?>
