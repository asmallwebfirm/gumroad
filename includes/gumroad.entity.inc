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
  public $title = '';
  public $gumroad_url = '';
  public $author_uid = NULL;
  public $created = '';
  public $modified = '';

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
    $bundle = gumroad_product_type_load($entity->type);
    $button = $bundle->settings['gumroad_link']['button'];
    $overlay = $bundle->settings['gumroad_link']['overlay'];
    $display = field_extra_fields_get_display('gumroad_product', $entity->type, $view_mode);

    // Attach JS if appropriate.
    if ($button && $display['gumroad_url']['visible']) {
      drupal_add_js(GUMROAD_BUTTON_JS, 'external');
    }
    if ($overlay && $display['gumroad_url']['visible']) {
      drupal_add_js(GUMROAD_OVERLAY_JS, 'external');
    }

    // Theme the Gumroad URL like a normal field.
    $class = $button ? array('gumroad-button') : array();
    $content['gumroad_url'] = array(
      '#theme' => 'field',
      '#weight' => 0,
      '#title' => t('Gumroad URL'),
      '#access' => TRUE,
      '#label_display' => 'hidden',
      '#view_mode' => $view_mode,
      '#language' => $langcode,
      '#field_name' => 'gumroad_url',
      '#field_type' => 'text',
      '#entity_type' => 'gumroad_product',
      '#bundle' => $entity->type,
      '#items' => array(array('value' => $entity->gumroad_url)),
      '#formatter' => 'text_default',
      0 => array(
        '#theme' => 'link',
        '#text' => $entity->title,
        '#path' => $entity->gumroad_url,
        '#options' => array(
          'attributes' => array('class' => $class),
          'html' => FALSE,
        ),
      ),
    );

    return parent::buildContent($entity, $view_mode, $langcode, $content);
  }
}


/**
 * Optionally improves Inline Entity Form UX.
 */
if (class_exists('EntityInlineEntityFormController')) {
  class GumroadProductInlineEntityFormController extends EntityInlineEntityFormController {
    /**
     * Provides entity-specific labels for use inline entity form.
     */
    public function labels() {
      return array(
        'singular' => t('product'),
        'plural' => t('products'),
      );
    }

    /**
     * Some useful fields to return for the default table.
     */
    public function defaultFields($bundles) {
      // Add the product title.
      $fields['title'] = array(
        'type' => 'property',
        'label' => t('Title'),
        'visible' => 1,
        'weight' => 0,
      );

      // Add the gumroad URL.
      $fields['gumroad_url'] = array(
        'type' => 'property',
        'label' => t('Gumroad URL'),
        'visible' => 1,
        'weight' => 1,
      );

      return $fields;
    }

    /**
     * Loads our entity form.
     */
    public function entityForm($entity_form, &$form_state) {
      module_load_include('inc', 'gumroad', 'includes/gumroad.forms');
      $product = $entity_form['#entity'];
      return gumroad_product_form($entity_form, $form_state, $product);
    }

    /**
     * Validates our entity form.
     */
    public function entityFormValidate($entity_form, &$form_state) {
      module_load_include('inc', 'gumroad', 'includes/gumroad.forms');
      $child_form_state = form_state_defaults();
      $child_form_state['values'] = drupal_array_get_nested_value($form_state['values'], $entity_form['#parents']);
      $child_form_state['gumroad_product'] = $entity_form['#entity'];
      gumroad_product_form_validate($entity_form, $child_form_state);
    }

    /**
     * Submits our entity form.
     */
    public function entityFormSubmit(&$entity_form, &$form_state) {
      parent::entityFormSubmit($entity_form, $form_state);

      module_load_include('inc', 'gumroad', 'includes/gumroad.forms');
      $child_form_state = form_state_defaults();
      $child_form_state['values'] = drupal_array_get_nested_value($form_state['values'], $entity_form['#parents']);
      $child_form_state['gumroad_product'] = $entity_form['#entity'];
      gumroad_product_form_submit($entity_form, $child_form_state);
    }
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
