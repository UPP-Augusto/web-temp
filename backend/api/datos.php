<?php

// Headers para CORS
header("Content-Type: application/json; charset=utf-8");
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type");

// Manejar preflight requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit();
}

// Leer y validar datos JSON
$input = file_get_contents('php://input');
$data = json_decode($input, true);

if (!$data) {
    http_response_code(400);
    echo json_encode(['error' => 'Datos JSON inválidos']);
    exit();
}

// Validar campos requeridos
if (empty($data['device_id']) || !isset($data['temperatura'])) {
    http_response_code(400);
    echo json_encode(['error' => 'Faltan campos requeridos: device_id y temperatura']);
    exit();
}

// Sanitizar datos
$device_id = trim($data['device_id']);
$temperatura = (float)$data['temperatura'];
$humedad = isset($data['humedad']) ? (float)$data['humedad'] : null;
$timestamp = isset($data['timestamp']) ? (int)$data['timestamp'] : time();
$rssi = isset($data['rssi']) ? (int)$data['rssi'] : null;

// Incluir configuración de base de datos
require_once '../config/database.php';

try {
    // Buscar o crear dispositivo
    $stmt = $conn->prepare("SELECT id FROM device WHERE mac = ?");
    $stmt->bind_param("s", $device_id);
    $stmt->execute();
    $stmt->store_result();
    
    $device_db_id = 0;
    
    if ($stmt->num_rows === 0) {
        // Registrar nuevo dispositivo automáticamente
        $nombre_default = "ESP_" . substr($device_id, -6);
        
        $stmt2 = $conn->prepare("INSERT INTO device (mac, nombre) VALUES (?, ?)");
        $stmt2->bind_param("ss", $device_id, $nombre_default);
        
        if ($stmt2->execute()) {
            $device_db_id = $conn->insert_id;
            error_log("Dispositivo auto-registrado: $device_id -> ID: $device_db_id");
        } else {
            throw new Exception("Error al registrar dispositivo");
        }
        $stmt2->close();
    } else {
        $stmt->bind_result($device_db_id);
        $stmt->fetch();
    }
    $stmt->close();
    
    if ($device_db_id === 0) {
        throw new Exception("ID de dispositivo inválido");
    }
    
    // Insertar medición
    if ($humedad !== null) {
        $stmt = $conn->prepare("INSERT INTO measurement (device_id, temperatura, humedad) VALUES (?, ?, ?)");
        $stmt->bind_param("idd", $device_db_id, $temperatura, $humedad);
    } else {
        $stmt = $conn->prepare("INSERT INTO measurement (device_id, temperatura) VALUES (?, ?)");
        $stmt->bind_param("id", $device_db_id, $temperatura);
    }
    
    if ($stmt->execute()) {
        $medicion_id = $conn->insert_id;
        
        // Respuesta exitosa
        echo json_encode([
            'success' => true,
            'message' => 'Datos almacenados correctamente',
            'measurement_id' => $medicion_id,
            'device_id' => $device_db_id,
            'received' => [
                'temperatura' => $temperatura,
                'humedad' => $humedad,
                'timestamp' => $timestamp
            ]
        ]);
    } else {
        throw new Exception("Error al insertar medición");
    }
    
    $stmt->close();
    
} catch (Exception $e) {
    http_response_code(500);
    echo json_encode([
        'error' => 'Error al procesar datos',
        'message' => $e->getMessage()
    ]);
    
    // Log del error
    error_log("Error en datos.php: " . $e->getMessage());
    error_log("Datos recibidos: " . json_encode($data));
} finally {
    if (isset($conn)) {
        $conn->close();
    }
}
?>
