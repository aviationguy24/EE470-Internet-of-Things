<?php
$host     = 'localhost'; 
$dbname   = 'u515418176_AaronEstrada';
$username = 'u515418176_dbAaronEstrada';
$password = 'Mechanical2003!'; 

try {
  $conn = new PDO("mysql:host=$host;dbname=$dbname;charset=utf8mb4", $username, $password, [
    PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
    PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC
  ]);
  
} catch (PDOException $e) {
  die("Connection failed: " . $e->getMessage());
}
