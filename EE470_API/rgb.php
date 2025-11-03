
<?php
/**
 * ===============================================================
 *  EE470 – Internet of Things (IoT) Systems
 *  Project Part 2B: RGB LED Control
 * ===============================================================
 *  Student:    Aaron Estrada
 *  Institution:   Sonoma State University
 *  Course:        EE470 – Internet of Things
 *  Instructor:    Dr. Farid Farahmand
 * ---------------------------------------------------------------
 *  Description:
 *  This PHP script provides a web-based RGB LED controller interface
 *  that allows users to set an intensity value (0–255) for the red
 *  channel of an RGB LED connected to an ESP8266 NodeMCU.
 *
 *  The script:
 *    • Reads the current LED state from results.txt (ON/OFF)
 *    • Reads and updates RGB intensity values in rgb.txt
 *    • Displays both statuses with an Air Force–themed interface
 *    • Allows input via slider, numeric box, or URL parameter (?val=)
 *
 *  Integration:
 *    • Used with Part 2A (results.php) and Part 2C (ESP combined code)
 *    • Files: rgb.php, rgb.txt, results.txt
 *
 * ===============================================================
 */
 
/* EE470 Part 2B – RGB slider page */
$rgbFile = __DIR__ . '/rgb.txt';
$ledFile = __DIR__ . '/results.txt';
function rfile($p,$fb){ return file_exists($p)? trim(file_get_contents($p)) : $fb; }

$curLed = 'off';
if (file_exists($ledFile)) {
  $j = json_decode(rfile($ledFile,'{"led":"off"}'), true);
  $curLed = isset($j['led']) ? $j['led'] : 'off';
}
$curRGB = rfile($rgbFile,'0');

if ($_SERVER['REQUEST_METHOD']==='POST') {
  $v = isset($_POST['rgb']) ? intval($_POST['rgb']) : intval($_REQUEST['val'] ?? $curRGB);
  $v = max(0, min(255, $v));
  file_put_contents($rgbFile, $v.PHP_EOL, LOCK_EX);
  $curRGB = (string)$v;
}
?>
<!doctype html><html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>EE470 – RGB Control</title>
<style>
 body{font-family:system-ui,Arial;margin:24px;max-width:720px}
 .card{border:1px solid #ddd;border-radius:12px;padding:16px;margin:12px 0}
 .pill{padding:4px 10px;border-radius:999px;background:#eee;display:inline-block}
</style></head><body>
<h1>RGB Control (0–255)</h1>

<div class="card">
  <div>LED (from <code>results.txt</code>): <span class="pill"><?=htmlspecialchars($curLed)?></span></div>
  <div style="margin-top:6px">RGB value (from <code>rgb.txt</code>): <span class="pill"><?=htmlspecialchars($curRGB)?></span></div>
</div>

<form method="POST" class="card">
  <label for="rgb">Red channel</label><br>
  <input type="range" id="rgb" name="rgb" min="0" max="255" value="<?=htmlspecialchars($curRGB)?>">
  <div style="margin-top:8px">
    <input type="number" min="0" max="255" value="<?=htmlspecialchars($curRGB)?>"
           oninput="document.getElementById('rgb').value=this.value">
    <button type="submit">Save</button>
  </div>
</form>

<p>Tip: you can also set with <code>?val=180</code> in the URL.</p>
</body></html>
