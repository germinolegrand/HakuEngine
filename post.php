<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Moteur de recherche </title>
</head>

<body>
<span> Power Search Engine</span>
<form id="searchForm" name="moteurSubmit" method="GET" action="">
	<label for="q">Rechercher :</label>
	<input type="text" value="<?php if(isset($_GET['q'])) { echo htmlspecialchars($_GET['q']); } ?>" name="q"  />
	<input type="submit" value="Envoyer" />
</form>


<?php

include_once("class/BDD_connector.php");
include_once("class/Classes.moteur.php");

if(isset($_GET['q'])) { 
	$champ_entier = trim($_GET['q']);

	if(strlen($champ_entier)<= 2 && strlen($champ_entier) > 0 ){

				echo "Le mot est trop court!";

	}elseif(strlen($champ_entier)== 0){

				echo "Entrez un mot ! ";
	}elseif(strlen($champ_entier)>2){
				$k=0;
				$champ = explode(" ", $champ_entier);
				while(!empty($champ[$k])){
					echo $champ[$k];
					echo"<br>";
					$query = "SELECT score FROM keywords WHERE keyword = '$champ[$k]' ORDER BY score DESC ";
					$result = pg_query($dbconn,$query)or die("can not ".$query) ;
					$num = pg_num_fields($result);
					echo "$num";

					if($num != 0){
										while ($row = pg_fetch_row($result)) {
											++$i;
											  echo "Résultat $i: $row[0]   ";
											  echo "<br />\n";
											  
										}


									  
					}elseif($num == 0){
						$champ[$k] = "";
					}

					++$k;
					}
				}

	}


//echo pg_dbname();

			
?>
