<?php

/**
 * @file
 * CRUD operations for gumroad products and types.
 */


/**
 * Load a gumroad product.
 */
function gumroad_product_load($gumid, $reset = FALSE) {
  $products = gumroad_product_load_multiple(array($gumid), array(), $reset);
  return reset($products);
}


/**
 * Load multiple gumroad products based on certain conditions.
 */
function gumroad_product_load_multiple($gumids = array(), $conditions = array(), $reset = FALSE) {
  return entity_load('gumroad_product', $gumids, $conditions, $reset);
}


/**
 * Save a gumroad product.
 */
function gumroad_product_save($product) {
  entity_save('gumroad_product', $product);

  // Handle path aliases.
  if (isset($product->product_path)) {
    $path = $product->product_path;
    $path['alias'] = trim($path['alias']);

    // Only save a non-empty alias.
    if (!empty($path['alias'])) {
      // Ensure fields for programmatic executions.
      $langcode = function_exists('entity_language') ? entity_language('gumroad_product', $product) : NULL;
      $path['source'] = 'gumroad/' . entity_id('gumroad_product', $product);
      $path['language'] = isset($langcode) ? $langcode : LANGUAGE_NONE;
      path_save($path);
    }
    elseif (empty($path['alias']) && !empty($path['pid'])) {
      path_delete($path['pid']);
    }
  }
}


/**
 * Delete single gumroad product.
 */
function gumroad_product_delete($product) {
  gumroad_product_delete_multiple(array(entity_id('gumroad_product', $product)));
}


/**
 * Delete multiple gumroad products.
 */
function gumroad_product_delete_multiple($gumids) {
  // Handle path aliases.
  foreach ($gumids as $gumid) {
    path_delete(array('source' => 'gumroad/' . $gumid));
  }

  entity_delete_multiple('gumroad_product', $gumids);
}


/**
 * Load gumroad product type.
 */
function gumroad_product_type_load($product_type) {
  return gumroad_product_types($product_type);
}


/**
 * List of gumroad product types.
 */
function gumroad_product_types($type_name = NULL) {
  $types = entity_load_multiple_by_name('gumroad_product_type', isset($type_name) ? array($type_name) : FALSE);
  return isset($type_name) ? reset($types) : $types;
}


/**
 * Save gumroad product type entity.
 */
function gumroad_product_type_save($product_type) {
  entity_save('gumroad_product_type', $product_type);
}


/**
 * Delete single gumroad product type.
 */
function gumroad_product_type_delete($product_type) {
  entity_delete('gumroad_product_type', entity_id('gumroad_product_type' ,$product_type));
}


/**
 * Delete multiple gumroad product types.
 */
function gumroad_product_type_delete_multiple($product_type_ids) {
  entity_delete_multiple('gumroad_product_type', $product_type_ids);
}
