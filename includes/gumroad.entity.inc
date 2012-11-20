<?php

/**
 * @file
 * Classes for gumroad products and gumroad product types.
 */


/**
 * The class used for gumroad products.
 */
class GumroadProduct extends Entity {
  public $gumid = NULL;
  public $author_uid = NULL;
  public $gumroad_url = '';
  public $webhook_url = '';
  public $created = '';
  public $updated = '';

  public function __construct(array $values = array()) {
    // New product.
    if (!$this->gumid) {
      global $user;
      $this->created = REQUEST_TIME;
      $this->author_uid = $user->uid;
    }

    parent::__construct($values, 'gumroad_product');
  }

  protected function defaultLabel() {
    return $this->title;
  }

  protected function defaultUri() {
    return array('path' => 'gumroad/' . $this->identifier());
  }
}


/**
 *
 */
class GumroadProductController extends EntityAPIController {
  public function create(array $values = array()) {
    global $user;
    $values += array(
      'title' => '',
      'description' => '',
      'created' => REQUEST_TIME,
      'changed' => REQUEST_TIME,
      'uid' => $user->uid,
    );
    return parent::create($values);
  }

  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    return parent::buildContent($entity, $view_mode, $langcode, $content);
  }
}


/**
 *
 */
class GumroadProductTypeController extends EntityAPIControllerExportable {
   public function create(array $values = array()) {
    $values += array(
      'label' => '',
      'description' => '',
    );
    return parent::create($values);
  }

  /**
   * Save gumroad product type.
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    parent::save($entity, $transaction);
    // Rebuild menu registry. We do not call menu_rebuild directly, but set
    // variable that indicates rebuild in the end.
    // @see http://drupal.org/node/1399618
    variable_set('menu_rebuild_needed', TRUE);
  }
}

/**
 * UI controller for gumroad product type.
 */
class GumroadProductTypeUIController extends EntityDefaultUIController {
  /**
   * Overrides hook_menu() defaults.
   */
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['description'] = 'Manage Gumroad product types.';
    return $items;
  }
}


/**
 * Gumroad product type class.
 */
class GumroadProductType extends Entity {
  public $type;
  public $label;
  public $weight = 0;

  public function __construct($values = array()) {
    parent::__construct($values, 'gumroad_product_type');
  }

  function isLocked() {
    return isset($this->status) && empty($this->is_new) && (($this->status & ENTITY_IN_CODE) || ($this->status & ENTITY_FIXED));
  }
}
