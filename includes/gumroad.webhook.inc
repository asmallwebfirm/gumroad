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

  // Determine if this is a ping from Gumroad's webhook API.
  if (!empty($webhook_params)) {
    // Load this product type's settings.
    $type = gumroad_product_type_load($product->type);
    $settings = $type->settings;

    // If webhook processing is enabled for this product type, perform it.
    if (isset($settings['webhook']['webhook_status']) && $settings['webhook']['webhook_status']) {
      // Allow other modules to react to a Gumroad webhook call.
      foreach (module_implements('gumroad_webhook_reaction') as $module) {
        $url = module_invoke($module, 'gumroad_webhook_reaction', $product, $webhook_params);

        // If the module implementation returned a URL, we can exit the loop.
        if (!empty($url)) {
          break;
        }
      }

      // If no module returned a URL, continue processing according to settings.
      if (empty($url)) {
        switch ($settings['webhook']['webhook_enabled']['webhook_type']) {
          // Default webhook handling.
          case 'default':
            $e_url = entity_uri('gumroad_product', $product);
            $e_url['options']['absolute'] = TRUE;
            $url = url($e_url['path'], $e_url['options']);
            break;

          // Static webhook handling.
          case 'static':
            $url = $settings['webhook']['webhook_enabled']['webhook_url'];
            break;

          // Field-based webhook handling.
          case 'field':
            $wrapper = entity_metadata_wrapper('gumroad_product', $product);
            $field = $settings['webhook']['webhook_enabled']['webhook_field'];

            // Reactions based on the specified field type.
            switch ($wrapper->$field->type()) {
              // Handling for files.
              case 'field_item_file':
                $file = $wrapper->$field->value();
                $url = file_create_url($file['uri']);
                break;

              case 'field_item_link':
                $link = $wrapper->$field->value();
                $url = $link['url'];
                break;
            }
            break;
        }
      }
    }
    // If webhook processing is not enabled, something is misconfigured. We'll
    // log the error and return the path to the product entity as a backup.
    else {
      $e_url = entity_uri('gumroad_product', $product);
      $e_url['options']['absolute'] = TRUE;
      $url = url($e_url['path'], $e_url['options']);

      // Log the error state.
      watchdog('gumroad', 'Webhook ping from Gumroad was received on !product, but products of type @type do not have webhook processing enabled.<br /><br />You must either configure this product type for webhook processing or uncheck "Webhook" in the Gumroad interface for this product.', array(
        '!product' => l('gumroad/' . entity_id('gumroad_product', $product), entity_label('gumroad_product', $product)),
        '@type' => entity_label('gumroad_product_type', $type),
      ), WATCHDOG_ERROR);
    }

    // At this point, we have a $url to return to Gumroad's API. 
    header('Content-type: text/plain');
    echo $url;

    // Perform normal Drupal shutdown operations and exit.
    module_invoke_all('exit');
    exit();
  }
}
