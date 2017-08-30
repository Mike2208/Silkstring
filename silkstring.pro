TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    tls_connection.cpp \
    network_connection.cpp \
    vector_t.cpp \
    linux_network_socket.cpp \
    error_exception.cpp \
    crypto_x509_certificate.cpp \
    crypto_x509_private_key.cpp \
    crypto_x509_certificate_chain.cpp \
    string_user.cpp \
    string_user_admin.cpp \
    string_connection.cpp \
    secure_connection.cpp \
    string_connection_option.cpp \
    hardware_connection_option.cpp \
    crypto_tls_session.cpp \
    string_tls_connection.cpp \
    string_server.cpp \
    string_connection_functions.cpp \
    crypto_x509_id.cpp \
    user_cert_id.cpp \
    string_user_id.cpp \
    protocol_network_connection.cpp \
    protocol_vector.cpp \
    protocol_map.cpp \
    protocol_data.cpp \
    map_type.cpp \
    message_queue.cpp \
    thread_message_queue.cpp \
    thread_function.cpp \
    thread_queued.cpp \
    thread_module_manager.cpp \
    thread_module_manager_multi_message.cpp \
    silkstring_message.cpp \
    protocol_send_handle.cpp \
    protocol_thread.cpp \
    testvariadic.cpp \
    dynamic_pointer.cpp \
    protocol_module_instantiator.cpp \
    protocol_manager.cpp \
    user_manager.cpp \
    user_io_thread.cpp \
    global_message_queue_thread.cpp \
    string_user_register_module.cpp \
    string_memory.cpp \
    string_user_storage.cpp \
    test_functions.cpp \
    user_cli_input_thread.cpp \
    network_dummy_connection.cpp \
    protocol_messages.cpp \
    protocol_thread_memory.cpp \
    protocol_module_connection_state.cpp \
    protocol_module_certificate_manager.cpp \
    protocol_module_tls_connection.cpp \
    protocol_connection_module.cpp \
    user_cli_thread_messages.cpp \
    user_cli_thread_module_parser.cpp \
    user_cli_thread_module_register.cpp \
    user_cli_thread.cpp \
    string_thread.cpp \
    crypto_tls_certificate_credentials.cpp \
    string_thread_message.cpp \
    container_functions.cpp \
    multi_container.cpp

HEADERS += \
    typesafe_class.h \
    tls_connection.h \
    network_connection.h \
    vector_t.h \
    linux_network_socket.h \
    error_exception.h \
    crypto_x509_certificate.h \
    crypto_x509_private_key.h \
    crypto_x509_certificate_chain.h \
    string_user.h \
    crypto_header.h \
    string_user_admin.h \
    string_connection.h \
    secure_connection.h \
    string_connection_option.h \
    external_types.h \
    hardware_connection_option.h \
    string_protocols.h \
    hardware_protocols.h \
    crypto_tls_session.h \
    string_tls_connection.h \
    string_server.h \
    string_connection_functions.h \
    crypto_x509_id.h \
    user_cert_id.h \
    string_user_id.h \
    protocol_network_connection.h \
    protocol_vector.h \
    protocol_map.h \
    protocol_data.h \
    map_type.h \
    message_queue.h \
    thread_message_queue.h \
    thread_function.h \
    thread_queued.h \
    thread_module_manager.h \
    thread_module_manager_multi_message.h \
    silkstring_message.h \
    protocol_send_handle.h \
    protocol_thread.h \
    testvariadic.h \
    dynamic_pointer.h \
    protocol_module_instantiator.h \
    protocol_manager.h \
    user_manager.h \
    user_io_thread.h \
    user_io_messages.h \
    global_message_queue_thread.h \
    string_user_storage.h \
    test_functions.h \
    user_cli_input_thread.h \
    network_dummy_connection.h \
    protocol_messages.h \
    protocol_thread_memory.h \
    protocol_module_connection_state.h \
    protocol_module_certificate_manager.h \
    protocol_module_tls_connection.h \
    protocol_connection_module.h \
    debug_flag.h \
    user_cli_thread_data.h \
    user_cli_thread_module_parser.h \
    user_cli_thread_messages.h \
    user_cli_thread_module_register.h \
    user_cli_thread.h \
    string_thread.h \
    string_thread_memory.h \
    string_thread_module_user_register.h \
    crypto_tls_certificate_credentials.h \
    string_thread_message.h \
    container_functions.h \
    string_user_admin_declaration.h \
    multi_container.h \
    testing_class_declaration.h

unix|win32: LIBS += -lgnutls

unix|win32: LIBS += -lpthread
