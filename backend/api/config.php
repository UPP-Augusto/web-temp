<?php
// config.php - API de configuración para dispositivos ESP32

// Headers para CORS
header("Content-Type: application/json; charset=utf-8");
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: GET, POST, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type, Authorization");

// Manejar preflight requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit();
}

// Verificar que se proporcionó device_id
$device_id = $_GET['device_id'] ?? '';

if (empty($device_id)) {
    http_response_code(400);
    echo json_encode(['error' => 'device_id requerido']);
    exit();
}

// Incluir configuración de base de datos
require_once '../config/database.php';

try {
    // Buscar dispositivo por MAC
    $stmt = $conn->prepare("SELECT id FROM device WHERE mac = ?");
    $stmt->bind_param("s", $device_id);
    $stmt->execute();
    $stmt->store_result();
    
    $device_db_id = 0;
    
    if ($stmt->num_rows === 0) {
        // Registrar nuevo dispositivo
        $nombre_default = "ESP_" . substr($device_id, -6); // Últimos 6 caracteres
        
        $stmt2 = $conn->prepare("INSERT INTO device (mac, nombre) VALUES (?, ?)");
        $stmt2->bind_param("ss", $device_id, $nombre_default);
        
        if ($stmt2->execute()) {
            $device_db_id = $conn->insert_id;
            error_log("Nuevo dispositivo registrado: $device_id -> ID: $device_db_id");
        }
        $stmt2->close();
    } else {
        $stmt->bind_result($device_db_id);
        $stmt->fetch();
    }
    $stmt->close();
    
    if ($device_db_id === 0) {
        throw new Exception("Error al obtener/crear dispositivo");
    }
    
    // Obtener configuración del dispositivo
    $config = [];
    
    $stmt = $conn->prepare("SELECT clave, valor FROM config WHERE device_id = ?");
    $stmt->bind_param("i", $device_db_id);
    $stmt->execute();
    $result = $stmt->get_result();
    
    while ($row = $result->fetch_assoc()) {
        $valor = $row['valor'];
        // Convertir a número si es numérico
        if (is_numeric($valor)) {
            $config[$row['clave']] = (strpos($valor, '.') !== false) ? 
                                     (float)$valor : (int)$valor;
        } else {
            $config[$row['clave']] = $valor;
        }
    }
    $stmt->close();
    
    // Configuración por defecto si no existe
    $config_defaults = [
        'intervalo' => 30,
        'temp_alerta' => 35.0,
        'nombre' => "ESP_" . substr($device_id, -6),
        'version' => "1.0.0",
        'ultima_actualizacion' => date('Y-m-d H:i:s')
    ];
    
    foreach ($config_defaults as $clave => $valor) {
        if (!isset($config[$clave])) {
            $config[$clave] = $valor;
        }
    }
    
    // Respuesta exitosa
    echo json_encode([
        'success' => true,
        'device_id' => $device_id,
        'device_db_id' => $device_db_id,
        'config' => $config,
        'timestamp' => time()
    ]);
    
} catch (Exception $e) {
    http_response_code(500);
    echo json_encode([
        'error' => 'Error interno del servidor',
        'message' => $e->getMessage()
    ]);
} finally {
    if (isset($conn)) {
        $conn->close();
    }
}

