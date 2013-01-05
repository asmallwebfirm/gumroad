<?php

/**
 * @file
 * All functions and hooks related to Gumroad webhook API processing.
 */


/**
 * Primary webhook handler.
 *
 * @see gumroad_product_view()
 */
function gumroad_webhook_handling($product) {
  $request_body = file_get_contents('php://input');
  parse_str($request_body, $webhook_params);

  // If there's request body data, assume it's coming from Gumroad.
  // @todo There must be a better way to verify that it's coming from Gumroad.
  if (!empty($webhook_params)) {
    // Load this product type's settings.
    $type = gumroad_product_type_load($product->type);
    $settings = $type->settings;

    // If webhook processing is enabled for this product type, perform it.
    if (isset($settings['webhook']['webhook_status']) && $settings['webhook']['webhook_status']) {
      // Allow other modules to react to a Gumroad webhook call.
      foreach (module_implements('gumroad_webhook_reaction') as $module) {
        $urls[$module] = module_invoke($module, 'gumroad_webhook_reaction', $product, $settings, $webhook_params);
      }

      // Return a URL to Gumroad based on the content type settings.
      switch ($settings['webhook']['webhook_enabled']['webhook_type']) {
        // Default webhook handling.
        case 'default':
          $e_url = entity_uri('gumroad_product', $product);
          $e_url['options']['absolute'] = TRUE;
          $url = url($e_url['path'], $e_url['options']);
          $error_details = format_string('Check !issue_queues for potential problems.', array(
            '!issue_queues' => l('the Gumroad module issue queues', 'http://www.drupal.org/project/issues/gumroad'),
          ));
          break;

        // Static webhook handling.
        case 'static':
          $url = $settings['webhook']['webhook_enabled']['webhook_url'];
          $error_details = format_string('Check that the webhook API static URL is properly set on !product_type.', array(
            '!product_type' => l('the Gumroad product type', 'admin/structure/gumroad-product-types/manage/' . $product->type),
          ));
          break;

        // Field-based webhook handling.
        case 'field':
          $wrapper = entity_metadata_wrapper('gumroad_product', $product);
          $field = $settings['webhook']['webhook_enabled']['webhook_field'];
          $field_info = field_info_field($field);
          $field_instance_info = field_info_instance('gumroad_product', $field, $product->type);
          $error_details = format_string('Check that the %field field has content.', array(
            '%field' => isset($field_instance_info['label']) ? $field_instance_info['label'] : $field,
          ));

          // Reactions based on the specified field type.
          switch ($field_info['type']) {
            // Handling for files.
            case 'file':
              $file = $wrapper->$field->value();
              $url = file_create_url($file['uri']);
              break;

            case 'link_field':
              $link = $wrapper->$field->value();
              $url = $link['url'];
              break;
          }
          break;

        // Return the URL returned by the specified module.
        case 'module':
          $url = $urls[$settings['webhook']['webhook_enabled']['webhook_module']];
          $module_info = system_get_info('module', $settings['webhook']['webhook_enabled']['webhook_module']);
          $error_details = format_string('Check that the @module module is enabled and working properly.', array(
            '@module' => $module_info['name'],
          ));
          break;
      }

      // If a URL was returned, pass it back to Gumroad.
      if (!empty($url)) {
        header('Content-type: text/plain');
        echo $url;

        // Perform normal Drupal shutdown operations and exit.
        module_invoke_all('exit');
        exit();
      }
      // Otherwise, there was a problem and it should be logged.
      else {
        watchdog('gumroad', 'Webhook API call was received on !product, but no URL was found to hand off to Gumroad. !details', array(
          '!product' => l(entity_label('gumroad_product', $product), 'gumroad/' . entity_id('gumroad_product', $product)),
          '!details' => $error_details,
        ), WATCHDOG_ERROR);
      }
    }
    // If webhook processing is not enabled, Gumroad is misconfigured.
    else {
      // Log the error state.
      watchdog('gumroad', 'Webhook API call was received on !product, but products of type @type do not have webhook processing enabled.<br /><br />You must either configure this product type for webhook processing or uncheck "Webhook" in the Gumroad interface for this product.', array(
        '!product' => l('gumroad/' . entity_id('gumroad_product', $product), entity_label('gumroad_product', $product)),
        '@type' => entity_label('gumroad_product_type', $type),
      ), WATCHDOG_ERROR);
    }
  }

  // If we're here and the request method is POST, we have to recheck the user's
  // access privileges. For details, @see gumroad_access()
  if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    global $user;

    // If the user has neither of these permissions, they do not have access.
    if (!user_access('administer gumroad products', $user) && !user_access('view gumroad products', $user)) {
      drupal_access_denied();
    }
  }
}
