<?php
include('config.php');

// Select only one node (example: node_1)
$query = "SELECT node_name, temperature, time_received 
          FROM sensor_data 
          WHERE node_name='node_1' 
          ORDER BY time_received ASC";

$result = $conn->query($query);

// Convert to JSON
$data = [];
while ($row = $result->fetch_assoc()) {
  $data[] = $row;
}
echo json_encode($data);

$conn->close();
?>
