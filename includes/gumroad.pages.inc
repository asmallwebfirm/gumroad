<?php

/**
 * @file
 * Page callbacks for the gumroad module.
 */


/**
 * Page callback for adding/editing gumroad products.
 */
function gumroad_product_add($type) {
  module_load_include('inc', 'gumroad', 'includes/gumroad.forms');

  $product_type = gumroad_product_types($type);
  $product = entity_get_controller('gumroad_product')->create(array('type' => $type));
  drupal_set_title(t('Create @name', array('@name' => entity_label('gumroad_product_type', $product_type))));

  return drupal_get_form('gumroad_product_form', $product);
}


/**
 * Page callback for viewing a single gumroad product.
 */
function gumroad_product_view($product) {
  // Handle all webhook processing where appropriate.
  module_load_include('inc', 'gumroad', 'includes/gumroad.webhook');
  gumroad_webhook_handling($product);

  // Display the entity.
  drupal_set_title(entity_label('gumroad_product', $product));
  return entity_view('gumroad_product', array(entity_id('gumroad_product', $product) => $product), 'full');
}


/**
 * Page callback to view all product types and jump to product add pages.
 */
function gumroad_add_page() {
  $types = gumroad_product_types();

  // If only one type is declared, redirect to that page.
  if (count($types) == 1) {
    $type = array_shift($types);
    drupal_goto('gumroad/add/' . $type->type);
  }
  // If no types are declared, display a message about creating product types.
  elseif (empty($types)) {
    $output = '<p>' . t('You have not created any Gumroad product types yet. Go to the <a href="@create-type">product type creation page</a> to add one.', array('@create-type' => url('admin/structure/gumroad-product-types/add'))) . '</p>';
  }
  else {
    $output = '<dl class="gumroad-product-type-list">';
    foreach ($types as $type) {
      $output .= '<dt>' . l($type->label, 'gumroad/add/' . $type->type) . '</dt>';
      $output .= '<dd>' . filter_xss_admin($type->description) . '</dd>';
    }
    $output .= '</dl>';
  }

  return $output;
}


/**
 * Page callback to view a list of all gumroad products.
 */
function gumroad_product_overview() {
  // Check if there is sorting request
  if (isset($_GET['sort']) && isset($_GET['order'])) {
    // Sort it Ascending or Descending?
    $sort = $_GET['sort'] == 'asc' ? 'ASC' : 'DESC';
 
    // Which column will be sorted
    switch($_GET['order']){
      case 'Title':
        $order = 'title';
        break;
      case 'Type':
        $order = 'type';
        break;
      case 'Author':
        $order = 'author_uid';
        break;
      case 'Gumroad URL':
        $order = 'gumroad_url';
        break;
      default:
        $order = 'modified';
    }
  }
  else{
    // Default sort
    $sort = 'ASC';
    $order = 'modified';
  }
  
  // Select table
  $query = db_select('gumroad_product', 'g');

  // Selected fields
  $query->fields('g', array('gumid', 'title', 'type', 'author_uid', 'gumroad_url', 'modified'));

  // Set order by.
  $query->orderBy($order, $sort);

  // Pagination.
  $query = $query->extend('TableSort')->extend('PagerDefault')->limit(50);
 
  // Execute query
  $result = $query->execute();
 
  // Prepare table header
  $header = array(
    array(
      'data' => t('Title'),
      'field' => 'title',
    ),
    array(
      'data' => t('Type'),
      'field' => 'type',
    ),
    array(
      'data' => t('Author'),
      'field' => 'author_uid',
    ),
    array(
      'data' => t('Gumroad URL'),
      'field' => 'gumroad_url',
    ),
    array(
      'data' => t('Updated'),
      'field' => 'modified',
    )
  );
 
  $rows = array();
  foreach ($result as $data) {
    // Get author details.
    $account = user_load($data->author_uid);
 
    // Get product type details.
    $type = gumroad_product_type_load($data->type);
 
    // Fill the table rows.
    $rows[] = array(
      l($data->title, 'gumroad/' . $data->gumid),
      $type->label,
      l($account->name, 'user/' . $data->author_uid),
      l($data->gumroad_url, $data->gumroad_url),
      date("m/d/Y - H:i", $data->modified),
    );
  }
 
  // Output of table with the paging
  $output = theme_table(
    array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array('width' => '100%'),
      'sticky' => true,
      'caption' => '',
      'colgroups' => array(),
      'empty' => t('No products yet.'),
    )
  ).theme('pager');
 
  return $output;
}
