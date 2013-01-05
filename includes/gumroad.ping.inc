<?php

/**
 * @file
 * All functions and hooks related to Gumroad Ping processing.
 */


/**
 * Primary ping handler.
 */
function gumroad_ping_handler() {
  $request_body = file_get_contents('php://input');
  parse_str($request_body, $ping_params);

  // If there's request body data, assume it's coming from Gumroad.
  // @todo There must be a better way to verify that it's coming from Gumroad.
  if (!empty($ping_params)) {
    // Allow other modules to react to a Gumroad Ping.
    module_invoke_all('gumroad_ping_reaction', $ping_params);

    // Log the ping in watchdog.
    watchdog('gumroad', 'Gumroad ping was received.', array(), WATCHDOG_NOTICE);

    // The Gumroad Ping API doesn't specify required response, so send an HTTP
    // 200 and be done with it.
    drupal_add_http_header('Status', '200 OK');

    // Perform normal Drupal shutdown operations and exit.
    module_invoke_all('exit');
    exit();
  }
}
