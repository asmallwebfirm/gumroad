<?php

/**
 * @file
 * Rules integration hooks/functions for the Gumroad module.
 */


/**
 * Implements hook_rules_data_info().
 */
function gumroad_rules_data_info() {
  return array(
    'gumroad_api' => array(
      'label' => t('gumroad api data'),
      'wrap' => TRUE,
      'property info' => array(
        'email' => array(
          'type' => 'text',
          'label' => t('The email of the customer'),
        ),
        'price' => array(
          'type' => 'decimal',
          'label' => t('The amount paid by the customer'),
        ),
        'variants' => array(
          'type' => 'list',
          'label' => t('The variants associated with the product purchased'),
        ),
      ),
    ),
  );
}


/**
 * Implements hook_rules_event_info().
 */
function gumroad_rules_event_info() {
  $defaults = array(
    'group' => t('gumroad product'),
    'module' => 'gumroad',
  );
  return array(
    'gumroad_webhook' => $defaults + array(
      'label' => t('After receiving a webhook API request'),
      'variables' => array(
        'gumroad_api' => array(
          'type' => 'gumroad_api',
          'label' => t('Gumroad API payload'),
          'description' => t('The data returned by the Gumroad API call, including e-mail address, price, etc.'),
        ),
        'gumroad_product' => array(
          'type' => 'gumroad_product',
          'label' => t('Purchased product'),
          'description' => t('The product purchased that trigger the webhook call.'),
        ),
      ),
    ),
    'gumroad_ping' => $defaults + array(
      'label' => t('After receiving a ping API request'),
      'variables' => array(
        'gumroad_api' => array(
          'type' => 'gumroad_api',
          'label' => t('Gumroad API payload'),
          'description' => t('The data returned by the Gumroad API call, including e-mail address, price, etc.'),
        ),
      ),
    ),
  );
}
