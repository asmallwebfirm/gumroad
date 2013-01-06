<?php

/**
 * @file
 * Gumroad product type context condition plugin.
 */


/**
 * Exposes Gumroad product types as a context condition.
 */
class gumroad_context_context_condition_gumroad_product_type extends context_condition {

  function condition_values() {
    $values = array();
 
    // Get product types and their names and return these as values for this
    // context condition
    module_load_include('inc', 'gumroad', 'includes/gumroad.crud');
    $types = gumroad_product_types();
 
    foreach($types as $machine_name => $type) {
      $values[$machine_name] = entity_label('gumroad_product_type', $type);
    }

    return $values;
  }
 
  function execute($product_type) {
    foreach ($this->get_contexts($product_type) as $context) {
      $this->condition_met($context, $product_type);
    }
  }
}
