<?php
require 'config.php'; // $conn = new PDO(...)

/* ---------- helpers ---------- */
$TEMP_MIN=-40; $TEMP_MAX=125; $HUM_MIN=0; $HUM_MAX=100;
function G($k){ return isset($_GET[$k]) ? trim($_GET[$k]) : null; }

/* ================================================================
   1) BASE64-ENCODED INSERT  (expects ?b=<base64>)
      Decodes strings like:
      "nodeId=node_3&nodeTemp=34&timeReceived=2025-10-10 20:25:01"
      Optional: nodeHum=55
   ================================================================ */
if (isset($_GET['b'])) {
  $decoded = base64_decode($_GET['b'], true);
  if ($decoded === false) {
    http_response_code(400);
    echo "Invalid Base64 data.";
    exit;
  }

  parse_str($decoded, $p); // parse into array
  $n  = $p['nodeId']        ?? null;
  $t  = $p['nodeTemp']      ?? null;
  $ts = $p['timeReceived']  ?? null;
  $h  = $p['nodeHum']       ?? 0;   // optional; default 0

  if (!$n || $t===null || !$ts) {
    http_response_code(400);
    echo "Missing required fields in decoded data. Got: ".htmlspecialchars($decoded);
    exit;
  }

  try {
    // node must be registered
    $ok = $conn->prepare("SELECT 1 FROM sensor_register WHERE node_name=?");
    $ok->execute([$n]);
    if (!$ok->fetchColumn()) throw new Exception("Node '$n' not registered.");

    if(!is_numeric($t)||$t<$TEMP_MIN||$t>$TEMP_MAX) throw new Exception('Temperature out of range.');
    if(!is_numeric($h)||$h<$HUM_MIN||$h>$HUM_MAX)   throw new Exception('Humidity out of range.');

    $ins = $conn->prepare("
      INSERT INTO sensor_data (node_name, time_received, temperature, humidity)
      VALUES (?, ?, ?, ?)
    ");
    $ins->execute([$n, $ts, $t, $h]);

    echo "BASE64 Data inserted successfully:<br>nodeId=$n<br>nodeTemp=$t<br>timeReceived=$ts<br>nodeHum=$h";
  } catch (PDOException $e) {
    // 23000 => duplicate (requires UNIQUE (node_name,time_received))
    if ($e->getCode()==='23000') {
      echo "Duplicate: ($n, $ts) already exists.";
    } else {
      echo "DB error: ".$e->getMessage();
    }
  } catch (Exception $e) {
    echo "Error: ".$e->getMessage();
  }
  exit; // stop after handling Base64
}

/* ================================================================
   2) INSERT via regular GET (?node_name=...&temperature=...&humidity=...[&time_received=...])
   ================================================================ */
$msg = '';
if (G('node_name') || G('node')) {
  $n  = G('node_name') ?: G('node');
  $t  = G('temperature') ?: G('temp');
  $h  = G('humidity')    ?: G('hum');
  $ts = G('time_received') ?: G('time'); // optional

  try {
    if ($n==='' || $t===null || $h===null) throw new Exception('Need node_name, temperature, humidity.');
    $ok = $conn->prepare("SELECT 1 FROM sensor_register WHERE node_name=?");
    $ok->execute([$n]);
    if (!$ok->fetchColumn()) throw new Exception("Node '$n' not registered.");
    if (!is_numeric($t)||$t<$TEMP_MIN||$t>$TEMP_MAX) throw new Exception('Temperature out of range.');
    if (!is_numeric($h)||$h<$HUM_MIN||$h>$HUM_MAX)   throw new Exception('Humidity out of range.');

    $ins = $conn->prepare("
      INSERT INTO sensor_data(node_name,time_received,temperature,humidity)
      VALUES(?, COALESCE(?, NOW()), ?, ?)
    ");
    $ins->execute([$n, ($ts ?: null), $t, $h]);

    $msg = "Inserted: $n @ ".($ts ?: '(server time)')."  T=$t  H=$h";
  } catch (PDOException $e) {
    $msg = ($e->getCode()==='23000')
      ? "Insert failed: duplicate timestamp for node '$n' at '".($ts ?: 'NOW()')."'."
      : "Insert failed: ".$e->getMessage();
  } catch (Exception $e) { $msg = "Insert failed: ".$e->getMessage(); }
}

/* ================================================================
   3) JSON endpoint
      - ?json=node_1  -> rows for that node
      - ?json=all     -> all rows
   ================================================================ */
if ($jn = G('json')) {
  header('Content-Type: application/json; charset=utf-8');
  if ($jn === 'all') {
    $q = $conn->query("SELECT node_name, temperature, humidity, time_received
                       FROM sensor_data ORDER BY node_name, time_received");
    echo json_encode($q->fetchAll(PDO::FETCH_ASSOC));
  } else {
    $q = $conn->prepare("SELECT node_name, temperature, humidity, time_received
                         FROM sensor_data WHERE node_name=? ORDER BY time_received ASC");
    $q->execute([$jn]);
    echo json_encode($q->fetchAll(PDO::FETCH_ASSOC));
  }
  exit;
}

/* ================================================================
   4) Page display (tables + chart)
   ================================================================ */
$reg = $conn->query("SELECT node_name, manufacturer, longitude, latitude
                     FROM sensor_register ORDER BY node_name")->fetchAll();

$rows = $conn->query("SELECT node_name, time_received, temperature, humidity
                      FROM sensor_data ORDER BY node_name, time_received")->fetchAll();

$avg_node = G('avg_node') ?: 'node_1';
$avgQ = $conn->prepare("SELECT AVG(temperature) atmp, AVG(humidity) ahum, COUNT(*) n
                        FROM sensor_data WHERE node_name=?");
$avgQ->execute([$avg_node]); $avg = $avgQ->fetch(PDO::FETCH_ASSOC);

$chart_node = G('chart_node') ?: 'node_1';
$cq = $conn->prepare("SELECT time_received, temperature
                      FROM sensor_data WHERE node_name=? ORDER BY time_received ASC");
$cq->execute([$chart_node]); $chartRows = $cq->fetchAll(PDO::FETCH_ASSOC);
$chartLabels = array_column($chartRows, 'time_received');
$chartTemps  = array_map('floatval', array_column($chartRows, 'temperature'));
?>
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>SSU IoT Lab</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<style>
  body{font-family:Arial;background:#f9f9f9;text-align:center}
  h1,h2{color:#333}
  table{margin:16px auto;border-collapse:collapse;width:90%}
  th,td{border:1px solid #ddd;padding:8px} th{background:#a6ce39;color:#fff}
  .msg{margin:10px;color:#222}
  #chartWrap{max-width:980px;margin:24px auto;background:#fff;padding:16px;border-radius:12px;box-shadow:0 8px 24px rgba(0,0,0,.06)}
  .note{color:#666;font-size:14px}
</style>
</head>
<body>
  <h1>Welcome to SSU IoT Lab</h1>

  <?php if($msg): ?><div class="msg"><?=htmlspecialchars($msg)?></div><?php endif; ?>

  <h2>Registered Sensor Nodes</h2>
  <table>
    <tr><th>Node Name</th><th>Manufacturer</th><th>Longitude</th><th>Latitude</th></tr>
    <?php foreach($reg as $r): ?>
      <tr>
        <td><?=htmlspecialchars($r['node_name'])?></td>
        <td><?=htmlspecialchars($r['manufacturer'])?></td>
        <td><?=htmlspecialchars($r['longitude'])?></td>
        <td><?=htmlspecialchars($r['latitude'])?></td>
      </tr>
    <?php endforeach; ?>
  </table>

  <h2>Data Received</h2>
  <table>
    <tr><th>Node Name</th><th>Time</th><th>Temperature</th><th>Humidity</th></tr>
    <?php if(!$rows): ?><tr><td colspan="4">No data.</td></tr><?php endif; ?>
    <?php foreach($rows as $d): ?>
      <tr>
        <td><?=htmlspecialchars($d['node_name'])?></td>
        <td><?=htmlspecialchars($d['time_received'])?></td>
        <td><?=htmlspecialchars($d['temperature'])?></td>
        <td><?=htmlspecialchars($d['humidity'])?></td>
      </tr>
    <?php endforeach; ?>
  </table>

  <p class="note">
    Average for <b><?=htmlspecialchars($avg_node)?></b> —
    Temp: <b><?= $avg['n']?number_format($avg['atmp'],2):'—' ?></b>,
    Humidity: <b><?= $avg['n']?number_format($avg['ahum'],2):'—' ?></b>
    </code>
  </p>

  <div id="chartWrap">
    <h2>Sensor Node <?=htmlspecialchars($chart_node)?> — Temperature vs Time</h2>
    <canvas id="tempChart"></canvas>
    
  </div>

<script>
const labels = <?= json_encode($chartLabels, JSON_UNESCAPED_SLASHES) ?>;
const temps  = <?= json_encode($chartTemps,  JSON_UNESCAPED_SLASHES) ?>;

new Chart(document.getElementById('tempChart').getContext('2d'), {
  type: 'bar', // change to 'line' to answer Q1
  data: {
    labels,
    datasets: [{
      label: 'Temperature (°C)',
      data: temps,
      backgroundColor: 'rgba(0, 200, 0, 0.35)', // green
      borderColor: 'rgba(0, 128, 0, 1)',
      borderWidth: 2,
      tension: 0.25,
      pointRadius: 2
    }]
  },
  options: {
    responsive: true,
    scales: {
      x: { title: { display: true, text: 'Time' } },
      y: { title: { display: true, text: 'Temperature (°C)' }, beginAtZero: false }
    }
  }
});
</script>

</body>
</html>

