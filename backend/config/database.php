<?php

// Configuración de la base de datos
define('DB_HOST', 'localhost');
define('DB_USER', 'root');
define('DB_PASS', ''); // Cambiar según tu configuración
define('DB_NAME', 'web_temp_db');
define('DB_CHARSET', 'utf8mb4');

/**
 * Establecer conexión a la base de datos
 * @return mysqli Objeto de conexión
 */
function conectarDB() {
    // Crear conexión
    $conn = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_NAME);
    
    // Verificar conexión
    if ($conn->connect_error) {
        // Log del error (no mostrar al cliente en producción)
        error_log("Error de conexión a BD: " . $conn->connect_error);
        
        // Respuesta genérica al cliente
        http_response_code(500);
        header('Content-Type: application/json');
        echo json_encode([
            'error' => 'Error interno del servidor',
            'message' => 'No se pudo conectar a la base de datos'
        ]);
        exit();
    }
    
    // Establecer charset
    if (!$conn->set_charset(DB_CHARSET)) {
        error_log("Error al establecer charset: " . $conn->error);
    }
    
    return $conn;
}

// Crear conexión global (opcional)
$conn = conectarDB();

// Configurar zona horaria si es necesario
date_default_timezone_set('America/Mexico_City'); // Cambiar según tu ubicación

// Función para cerrar conexión
function cerrarDB($conn) {
    if ($conn) {
        $conn->close();
    }
}

// Registrar cierre automático al final del script
register_shutdown_function(function() use ($conn) {
    if (isset($conn) && $conn instanceof mysqli) {
        $conn->close();
    }
});
?>
