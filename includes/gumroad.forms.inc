<?php

/**
 * @file
 * Hooks and functions related to administering gumroad products.
 */

/**
 * Generates the gumroad product type editing form.
 */
function gumroad_product_type_form($form, &$form_state, $product_type, $op = 'edit') {
  if ($op == 'clone') {
    $product_type->label .= ' (cloned)';
    $product_type->type = '';
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $product_type->label,
    '#description' => t('The human-readable name of this product type.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($product_type->type) ? $product_type->type : '',
    '#maxlength' => 32,
    '#disabled' => $product_type->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'gumroad_product_types',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this product type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#default_value' => isset($product_type->description) ? $product_type->description : '',
    '#description' => t('Description of this gumroad product type.'),
  );

  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
  );

  // Default button/overlay settings for this content type.
  $form['gumroad_link'] = array(
    '#type' => 'fieldset',
    '#title' => t('Button & Overlay'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
    '#description' => '<p>' . t("Configure how Gumroad URLs appear and behave for this product type.") . '</p>',
    '#tree' => TRUE,
  );

  $form['gumroad_link']['button'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Gumroad Button Styles'),
    '#default_value' => isset($product_type->settings['gumroad_link']['button']) ? $product_type->settings['gumroad_link']['button'] : FALSE,
  );
  $form['gumroad_link']['overlay'] = array(
    '#type' => 'checkbox',
    '#title' => t('Open links in overlay'),
    '#default_value' => isset($product_type->settings['gumroad_link']['overlay']) ? $product_type->settings['gumroad_link']['overlay'] : FALSE,
  );

  $form['webhook'] = array(
    '#type' => 'fieldset',
    '#title' => t('Webhook API'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
    '#description' => '<p>' . t("Configure how this product type interacts with Gumroad's webhook API.") . '</p>',
    '#tree' => TRUE,
  );
  $form['webhook']['webhook_status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Webhook enabled'),
    '#default_value' => isset($product_type->settings['webhook']['webhook_status']) ? $product_type->settings['webhook']['webhook_status'] : FALSE,
  );

  $form['webhook']['webhook_disabled'] = array(
    '#type' => 'fieldset',
    '#title' => t('Webhook disabled'),
    '#description' => t('When disabled, customers will simply be directed to the page specified in Gumroad after a successful transaction.'),
    '#states' => array(
      'visible' => array(
        ':input[name="webhook[webhook_status]"]' => array('checked' => FALSE),
      ),
    ),
  );

  $form['webhook']['webhook_enabled'] = array(
    '#type' => 'fieldset',
    '#title' => t('Webhook enabled'),
    '#description' => t('When enabled, point customers to the product page; after a successful transaction, customers will be directed dynamically based on the settings below.'),
    '#states' => array(
      'visible' => array(
        ':input[name="webhook[webhook_status]"]' => array('checked' => TRUE),
      ),
    ),
  );

  $form['webhook']['webhook_enabled']['webhook_type'] = array(
    '#type' => 'radios',
    '#title' => t('Handler type'),
    '#options' => array(
      'default' => t('None'),
      'static' => t('Static URL'),
      'field' => t('Field attached to this product type'),
    ),
    '#default_value' => isset($product_type->settings['webhook']['webhook_enabled']['webhook_type']) ? $product_type->settings['webhook']['webhook_enabled']['webhook_type'] : 'default',
  );

  $form['webhook']['webhook_enabled']['webhook_url'] = array(
    '#type' => 'textfield',
    '#length' => 255,
    '#title' => t('Static URL'),
    '#default_value' => isset($product_type->settings['webhook']['webhook_enabled']['webhook_url']) ? $product_type->settings['webhook']['webhook_enabled']['webhook_url'] : 'http://',
    '#description' => t('After a successful transaction, customers will be directed to this URL for all products of this type.'),
    '#states' => array(
      'visible' => array(
        ':input[name="webhook[webhook_enabled][webhook_type]"]' => array('value' => 'static'),
      ),
    ),
  );

  if (isset($product_type->type) && $options = _gumroad_get_webhook_field_fields($product_type->type)) {
    $form['webhook']['webhook_enabled']['webhook_field'] = array(
      '#type' => 'select',
      '#title' => t('Field'),
      '#options' => $options,
      '#default_value' => variable_get('gumroad_product_' . $product_type->type . '_webhook_field', ''),
      '#description' => t('After a successful transaction, customers will be directed to the file or link attached to products of this type.'),
      '#states' => array(
        'visible' => array(
          ':input[name="webhook[webhook_enabled][webhook_type]"]' => array('value' => 'field'),
        ),
      ),
    );
  }
  else {
    $form['webhook']['webhook_enabled']['webhook_field'] = array(
      '#type' => 'textfield',
      '#title' => t('Field'),
      '#description' => t('You must attach a file field or !link_field to this product type to use this option.', array('!link_field' => l('Link field', 'http://www.drupal.org/project/link'))),
      '#states' => array(
        'visible' => array(
          ':input[name="webhook[webhook_enabled][webhook_type]"]' => array('value' => 'field'),
        ),
      ),
      '#attributes' => array(
        'style' => 'display:none;',
      ),
    );
  }

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save product type'),
    '#weight' => 40,
  );

  if (!$product_type->isLocked() && $op != 'add' && $op != 'clone') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete product type'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('gumroad_product_type_form_submit_delete')
    );
  }

  return $form;
}


/**
 * Validation handler for gumroad_product_type_form().
 *
 * Consolidates all extra settings.
 */
function gumroad_product_type_form_validate($form, &$form_state) {
  $values = &$form_state['values'];
  $webhook = &$values['webhook'];
  $gumroad_link = &$values['gumroad_link'];

  // Ensure the webhook settings provided make sense.
  if ($webhook['webhook_status']) {
    switch ($webhook['webhook_enabled']['webhook_type']) {
      case 'static':
        if (!valid_url($webhook['webhook_enabled']['webhook_url']) || $webhook['webhook_enabled']['webhook_url'] == 'http://') {
          form_set_error('webhook[webhook_enabled][webhook_url', t('You must provide a valid URL when using the Static URL webhook handler type.'));
        }
        break;

      case 'field':
        if (empty($webhook['webhook_enabled']['webhook_field'])) {
          form_set_error('webhook[webhook_enabled][webhook_field', t('You must provide a valid field when using the field webhook handler type.'));
        }
        break;
    }

    // Add webhook settings to the settings array.
    $values['settings']['webhook'] = $webhook;
  }
  else {
    // If webhook processing is disabled, destroy the settings.
    $values['settings']['webhook'] = array();
  }

  // Add Button & Overlay settings.
  $values['settings']['gumroad_link'] = $gumroad_link;
}


/**
 * Submit handler for gumroad_product_type_form().
 */
function gumroad_product_type_form_submit($form, &$form_state) {
  $product_type = entity_ui_form_submit_build_entity($form, $form_state);

  // Save the product type.
  gumroad_product_type_save($product_type);

  // Redirect user back to list of task types.
  $form_state['redirect'] = 'admin/structure/gumroad-product-types';
}


/**
 * Generates the gumroad product type delete form.
 */
function gumroad_product_type_form_delete_confirm($form, &$form_state, $product_type) {
  $form_state['gumroad_product_type'] = $product_type;

  // Always provide entity id in the same form key as in the entity edit form.
  $form['gumroad_product_type_id'] = array('#type' => 'value', '#value' => entity_id('gumroad_product_type', $product_type));

  return confirm_form($form,
    t('Are you sure you want to delete product type %title?', array('%title' => entity_label('gumroad_product_type', $product_type))),
    'gumroad-product-type/' . entity_id('gumroad_product_type', $product_type),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}


/**
 * Submit handler for the delete action on gumroad_product_type_form().
 */
function gumroad_product_type_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/gumroad-product-types/manage/' . $form_state['gumroad_product_type']->type . '/delete';
}


/**
 * Returns a form array for adding/editing gumroad products.
 *
 * @todo Better documentation!
 */
function gumroad_product_form($form, &$form_state, $product) {
  $form_state['gumroad_product'] = $product;

  // Gumroad product title.
  $form['title'] = array(
    '#title' => t('Title'),
    '#type' => 'textfield',
    '#default_value' => isset($product->title) ? $product->title : '',
    '#description' => t('The name of this product.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  // Gumroad URL field.
  $form['gumroad_url'] = array(
    '#title' => t('Gumroad URL'),
    '#type' => 'textfield',
    '#default_value' => isset($product->gumroad_url) ? $product->gumroad_url : '',
    '#description' => t('The shortlink to the product on Gumroad, i.e. http://gum.co/foo.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  // Attach fields.
  field_attach_form('gumroad_product', $product, $form, $form_state);

  // Additional settings.
  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 50,
  );

  $path = array();
  if (!empty($product)) {
    $conditions = array('source' => 'gumroad/' . $product->gumid);
    $langcode = function_exists('entity_language') ? entity_language('gumroad_product', $product) : NULL;
    if ($langcode != LANGUAGE_NONE && !empty($langcode)) {
      $conditions['language'] = $langcode;
    }
    else {
      $conditions['language'] = LANGUAGE_NONE;
    }
    $path = path_load($conditions);
    if ($path === FALSE) {
      $path = array();
    }
  }
  $path += array(
    'pid' => NULL,
    'source' => isset($product->gumid) ? 'gumroad/' . $product->gumid : NULL,
    'alias' => '',
    'language' => isset($langcode) ? $langcode : LANGUAGE_NONE,
  );

  // Add path alias settings.
  $form['path'] = array(
    '#type' => 'fieldset',
    '#title' => t('URL path settings'),
    '#collapsible' => TRUE,
    '#collapsed' => empty($path['alias']),
    '#group' => 'additional_settings',
    '#weight' => 30,
    '#tree' => TRUE,
    '#element_validate' => array('gumroad_path_form_element_validate'),
  );
  $form['path']['alias'] = array(
    '#type' => 'textfield',
    '#title' => t('URL alias'),
    '#default_value' => $path['alias'],
    '#maxlength' => 255,
    '#description' => t('Optionally specify an alternative URL by which this product can be accessed. Use a relative path and don\'t add a trailing slash or the URL alias won\'t work.'),
  );
  $form['path']['pid'] = array('#type' => 'value', '#value' => $path['pid']);
  $form['path']['source'] = array('#type' => 'value', '#value' => $path['source']);
  $form['path']['language'] = array('#type' => 'value', '#value' => $path['language']);

  // Submit/delete buttons.
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save product'),
    '#weight' => 999,
  );
  $form['actions']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete product'),
    '#weight' => 1000,
    '#limit_validation_errors' => array(),
    '#submit' => array('gumroad_product_delete_form_submit')
  );

  return $form;
}


/**
 * Wrapper for the Path module element validation function of a similar name.
 */
function gumroad_path_form_element_validate($element, &$form_state, $complete_form) {
  if (function_exists('path_form_element_validate')) {
    path_form_element_validate($element, $form_state, $complete_form);
  }
  else {
    // Trim the submitted value.
    $alias = trim($form_state['values']['path']['alias']);
    if (!empty($alias)) {
      form_set_value($element['alias'], $alias, $form_state);
      if (isset($form_state['values']['language'])) {
        form_set_value($element['language'], $form_state['values']['language'], $form_state);
      }
      $path = $form_state['values']['path'];
      $query = db_select('url_alias')
        ->condition('alias', $path['alias'])
        ->condition('language', $path['language']);
      if (!empty($path['source'])) {
        $query->condition('source', $path['source'], '<>');
      }
      $query->addExpression('1');
      $query->range(0, 1);
      if ($query->execute()->fetchField()) {
        form_error($element, t('The alias is already in use.'));
      }
    }
  }
}


/**
 * Validation handler for gumroad_product_form().
 */
function gumroad_product_form_validate($form, &$form_state) {
  $product = $form_state['gumroad_product'];
  $product->modified = REQUEST_TIME;
  field_attach_form_validate('gumroad_product', $product, $form, $form_state);
}


/**
 * Submit handler for gumroad_product_form().
 */
function gumroad_product_form_submit($form, &$form_state) {
  $product = $form_state['gumroad_product'];
  entity_form_submit_build_entity('gumroad_product', $product, $form, $form_state);

  gumroad_product_save($product);

  if ($product->save()) {
    drupal_set_message(t('Product successfully saved.'));
    $form_state['redirect'] = array('gumroad/' . $product->gumid);
  }
  else {
    drupal_set_message(t('There has been an issue saving this product. Please contact the site administrator.'));
    watchdog('gumroad', 'Issue saving product %gumid', array('%gumid' => $product->gumid), WATCHDOG_NOTICE);
  }
}


/**
 * Form callback: confirmation form for deleting a gumroad product.
 */
function gumroad_product_delete_form($form, &$form_state, $product) {
  $form_state['gumroad_product'] = $product;

  $form = confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $product->label())),
    entity_uri('gumroad_product', $product),
    '<p>' . t('Deleting this product cannot be undone.') . '</p>',
    t('Delete'),
    t('Cancel'),
    'confirm'
  );

  return $form;
}


/**
 * Submit handler for gumroad_product_delete_form().
 */
function gumroad_product_delete_form_submit($form, &$form_state) {
  $product = $form_state['gumroad_product'];

  $title = $product->label();

  // No status returned from delete(), assume it worked.
  gumroad_product_delete_multiple(array($product->gumid));
  drupal_set_message(t('%title has been deleted.', array('%title' => $title)));
  watchdog('gumroad', 'Deleted product %title', array('%title' => $title), WATCHDOG_NOTICE);

  $form_state['redirect'] = '<front>';
}


/**
 * Returns a list of fields appropriate for use with field-type webhooks.
 */
function _gumroad_get_webhook_field_fields($product_type) {
  // Get all declared file and link fields.
  $useable_fields = array();
  foreach (field_info_fields() as $field => $field_info) {
    if ($field_info['type'] == 'file' || $field_info['type'] == 'link_field') {
      $useable_fields[] = $field;
    }
  }

  // Get all fields attached to the current product type.
  $attached_fields = field_info_instances('gumroad_product', $product_type);
  $intersection = array_intersect(array_keys($attached_fields), $useable_fields);
  if (!empty($intersection)) {
    foreach ($intersection as $field) {
      $options[$field] = $attached_fields[$field]['label'];
    }
  }
  else {
    $options = array();
  }


  return $options;
}
