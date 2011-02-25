<?php

// Contains UI management functions for the votingapi_actions module.

function votingapi_actions_admin_page() {
  $numSets = 25;

  votingapi_rebuild_action_cache();

  foreach (_votingapi_load_action_sets_from_db() as $set) {
    $vasid = $set['vasid'];
    $name = $set['name'];
    $links = array();
    $links['votingapi_actions_edit'] = array(
       'title' => theme('image', drupal_get_path('module', 'votingapi_actions') . '/icons/gtk-edit.png'),
       'html' => TRUE,
       'href' => "admin/settings/voting_actions/edit/$vasid",
       'attributes' => array('title' => t('Edit this action set')),
    );
    $links['votingapi_actions_export'] = array(
       'title' => theme('image', drupal_get_path('module', 'votingapi_actions') . '/icons/document-save.png'),
       'html' => TRUE,
       'href' => "admin/settings/voting_actions/export/$name",
       'attributes' => array('title' => t('Export this action set')),
    );
    $links['votingapi_actions_delete'] = array(
       'title' => theme('image', drupal_get_path('module', 'votingapi_actions') . '/icons/edit-delete.png'),
       'html' => TRUE,
       'href' => "admin/settings/voting_actions/delete/$name",
       'attributes' => array('title' => t('Delete this action set')),
    );
    $items[] = array($name, theme('links', $links));
    $used[$name] = true;
  }

  if (empty($items)) {
    $items[] = array('data' => array('<p class="description">' .t("No action sets have currently been defined. !click to create one.", array('!click' => l(t('Click here'), 'admin/settings/voting_actions/add'))) .'</p>'), 'colspan' => '2');
  }
  $output = theme('table', array(t('Name'), t('Actions')), $items, array("cellpadding" => "4"), t('User-Defined Action Sets'));
  $output .= theme('pager', NULL, $numSets, 1);

  $items = array();
  $default_actions = _votingapi_load_action_sets_from_modules();
  $set_status = variable_get('votingapi_action_status', array());

  foreach ($default_actions as $name => $set) {
    if ($used[$name]) {
      $status = t('Overridden');
    }
    else if (isset($set_status[$name])) {
      if ($set_status[$name]) {
        $status = 'Enabled';
      }
      else {
        $status = 'Disabled';
      }
    }
    else if ($set['enabled']) {
      $status = 'Enabled';
    }
    else {
      $status = 'Disabled';
    }

    if ($status == 'Enabled') {
      $links = array();
      $links['votingapi_actions_add'] = array(
         'title' => theme('image', drupal_get_path('module', 'votingapi_actions') . '/icons/edit-paste.png'),
         'html' => TRUE,
         'href' => "admin/settings/voting_actions/add/$name",
         'attributes' => array('title' => t('Clone this action set')),
      );
      $links['votingapi_actions_export'] = array(
         'title' => theme('image', drupal_get_path('module', 'votingapi_actions') . '/icons/document-save.png'),
         'html' => TRUE,
         'href' => "admin/settings/voting_actions/export/$name",
         'attributes' => array('title' => t('Export this action set')),
      );
      $links['votingapi_actions_disable'] = array(
         'title' => theme('image', drupal_get_path('module', 'votingapi_actions') . '/icons/process-stop.png'),
         'html' => TRUE,
         'href' => "admin/settings/voting_actions/disable/$name",
         'attributes' => array('title' => t('Disable this action set')),
      );
    }
    else if ($status == 'Disabled') {
      $links = array();
      $links['votingapi_actions_add'] = array(
         'title' => theme('image', drupal_get_path('module', 'votingapi_actions') . '/icons/edit-paste.png'),
         'html' => TRUE,
         'href' => "admin/settings/voting_actions/add/$name",
         'attributes' => array('title' => t('Clone this action set')),
      );
      $links['votingapi_actions_export'] = array(
         'title' => theme('image', drupal_get_path('module', 'votingapi_actions') . '/icons/document-save.png'),
         'html' => TRUE,
         'href' => "admin/settings/voting_actions/export/$name",
         'attributes' => array('title' => t('Export this action set')),
      );
      $links['votingapi_actions_enable'] = array(
         'title' => theme('image', drupal_get_path('module', 'votingapi_actions') . '/icons/media-playback-start.png'),
         'html' => TRUE,
         'href' => "admin/settings/voting_actions/enable/$name",
         'attributes' => array('title' => t('Enable this action set')),
      );
    }
    else if ($status == 'Overridden') {
      $links = array();
    }

    $items[] = array($name, $set['source'], $status, theme('links', $links));
  }

  if (empty($items)) {
    $items[] = array('data' => array('<p class="description">'. t('No default action sets are available.') .'</p>'), 'colspan' => '4');
  }
  $output .= theme('table', array(t('Name'), t('Source'), t('Status'), t('Actions')), $items, array("cellpadding" => "4"), t('Default Action Sets'));
  $output .= theme('pager', NULL, $numSets, 2);

  return $output;
}



function votingapi_actions_admin_import_page($set_name = NULL) {
  $form['set'] = array(
    '#type' => 'textarea',
    '#title' => t('Import action set'),
    '#rows' => 20
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t("Submit"),
  );

  return $form;
}
function votingapi_actions_admin_import_page_validate($form_id, &$form) {
  ob_start();
  eval($form['set']);
  ob_end_clean();

  if (is_array($sets)) {
    foreach($sets as $name => $set) {
      if ($name) {
        $errors = _votingapi_validate_action_set($name, $set);
        foreach($errors as $error) {
          form_set_error('', $error);
        }
      }
    }
  }
}

function votingapi_actions_admin_import_page_submit($formid, $form) {
  ob_start();
  eval($form['set']);
  ob_end_clean();

  if (is_array($sets)) {
    foreach($sets as $name => $set) {
      if ($name) {
        $errors = _votingapi_validate_action_set($name, $set);
        if (count($errors) == 0) {
          _votingapi_insert_set($name, $set);
          votingapi_rebuild_action_cache();
          drupal_goto('admin/settings/voting_actions');
        }
        else {
          foreach ($errors as $error) {
            drupal_set_message($error);
          }
        }
      }
      else {
        drupal_set_message(t('Unable to get an action set out of that.'));
        return;
      }
    }
  }
}


function votingapi_actions_admin_export_page($set_name) {
  $code = _votingapi_create_set_code($set_name);
  $lines = substr_count($code, "\n");
  $form['code'] = array(
    '#type' => 'textarea',
    '#title' => $set['name'],
    '#default_value' => $code,
    '#rows' => max($lines, 20)
  );
  return $form;
}

function votingapi_actions_admin_delete_page($set_name) {
  $form['set'] = array('#type' => 'value', '#value' => $set_name);
  return confirm_form(
    $form, 
    t('Are you sure you want to delete %title?', array('%title' => $set_name)), 
    $_GET['destination'] ? $_GET['destination'] : 'admin/settings/voting_actions', 
    t('This action cannot be undone.'),
    t('Delete'), t('Cancel')  
  );
}
function votingapi_actions_admin_delete_page_submit($formid, $form) {
  $name = $form['set'];
  $sets = _votingapi_load_action_sets_from_db();
  if ($sets[$name]) {
    _votingapi_delete_set($name, $set[$name]);
  }
  return 'admin/settings/voting_actions';
}


function votingapi_actions_admin_enable_page($set_name) {
  _votingapi_set_action_status($set_name, true);
  drupal_goto('admin/settings/voting_actions');
}

function votingapi_actions_admin_disable_page($set_name) {
  _votingapi_set_action_status($set_name, false);
  drupal_goto('admin/settings/voting_actions');
}

function _votingapi_create_set_code($set_name) {
  $data = cache_get('votingapi_action_sets');
  $action_sets = unserialize($data->data);
  $set = $action_sets[$set_name];
  if ($set) {
    unset($set['name']);
    $code = "\n\$sets = array(\n";
    $code .= "'$set_name' => ";
    ob_start();
    var_export($set);
    $code .= ob_get_clean();
    ob_end_clean();
    $code .= "\n);\n";
  }
  
  return $code;
}


function votingapi_actions_admin_add_page($vasid = NULL) {
  $set['actions'] = array();
  $set['conditions'] = array();
  $set['sets'] = array();

  return drupal_get_form('votingapi_actions_set_form', $set);
}

function votingapi_actions_admin_edit_page($vasid = NULL) {
  if (!empty($vasid)) {
    $set['vasid'] = $vasid;
  }
  else {
    $set['actions'] = array();
    $set['conditions'] = array();
    $set['sets'] = array();
  }

  return drupal_get_form('votingapi_actions_set_form', $set);
}

// Generate a one-off form for a specific voting action set.
function votingapi_actions_set_form($set, $form_values = NULL) {
  $form = array();
  $form['#multistep'] = TRUE;
  $form['#redirect'] = FALSE;
  $form['#tree'] = TRUE;

  // Here we look for incoming form values and modify the set as needed.
  if (isset($form_values)) {
    // Populate the set's conditions from the incoming form values.
    if (is_array($form_values['conditions']['existing'])) {
      foreach($form_values['conditions']['existing'] as $condition) {
        $set['conditions'][] = $action['value'];
      }
    }
    if ($form_values['op'] == t('Add condition')) {
      $set['conditions'][] = array('new' => TRUE, 'handler' => $form_values['conditions']['add']['condition_type']);
    }
    unset($form_values['conditions']['add']);

    // Populate the set's actions from the form values.
    if (is_array($form_values['actions']['existing'])) {
      foreach($form_values['actions']['existing'] as $action) {
        $set['actions'][] = $action['value'];
      }
    }
    if ($form_values['op'] == t('Add action')) {
      $set['actions'][] = $form_values['actions']['add']['action_type'];
    }
  }

  $form['vasid'] = array(
    '#type' => 'hidden',
    '#value' => $set['vasid'],
   );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Set name'),
    '#required' => TRUE,
    '#description' => t('A unique name for this set. Stick to lowercase letters, and use underscores instead of spaces.'),
    '#default_value' => $set['name'],
  );
  
  /*
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t("A description of this action set's operations. You'll appreciate filling this out later."),
    '#default_value' => $set['description'],
  );
  */
  
  if (isset($set['parent'])) {
    $form['parent'] = array(
      '#type' => 'hidden',
      '#value' => $set['parent'],
    );
    $form['required'] = array(
      '#type' => 'checkbox',
      '#title' => t('This set is required.'),
      '#return_value' => 1,
      '#default_value' => $set['required'],
      '#description' => t("If this box is checked, this set's conditions must be passed before the parent set's actions can execute."),
    );
  }
  else {
    $form['content_type'] = array(
      '#type' => 'hidden',
      '#value' => 'node',
    );

    /*
    $form['content_type'] = array(
      '#type' => 'select',
      '#title' => t('Content type'),
      '#options' => array('node' => t('Nodes'), 'comment' => t('Comments')),
      '#default_value' => isset($set['content_type']) ? $set['content_type'] : 'node',
      '#required' => TRUE,
      '#description' => t('The type of content this action set will apply to.')
    );
    */

    $form['required'] = array(
      '#type' => 'hidden',
      '#value' => TRUE,
    );
  }

  $form['weight'] = array(
    '#type' => 'hidden',
    '#value' => 0,
  );

  /*
  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => $set['weight'],
  );
  */

  $form['condition_mask'] = array(
    '#type' => 'select',
    '#title' => t("To execute this set's actions"),
    '#options' => array('AND' => t('...All conditions must be true'), 'OR' => t('...At least one condition must be true')),
    '#default_value' => isset($set['condition_mask']) ? $set['condition_mask'] : 'AND',
  );

  // Conditions
  $form['conditions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Conditions'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t('These conditions determine whether the actions in this set are executed.'),
  );

  foreach($set['conditions'] as $condition) {
    $form['conditions'][] = votingapi_actions_condition_form($condition);
  }

  $form['conditions']['add']['#weight'] = 100;
  $form['conditions']['add']['#theme'] = 'votingapi_actions_add_condition';
  $form['conditions']['add']['condition_type'] = array(
    '#type' => 'select',
    '#options' => _votingapi_actions_condition_types(),
  );
  $form['conditions']['add']['add_condition'] = array(
    '#type' => 'button',
    '#value' => t('Add condition'),
  );

  // Actions
  $form['actions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Actions'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t('If the conditions above pass, this list of actions will execute.'),
  );

  $form['actions']['existing']['#weight'] = 0;
  foreach($set['actions'] as $action) {
    $form['actions']['existing'][] = votingapi_actions_action_form($action);
  }

  $form['actions']['add']['#weight'] = 100;
  $form['actions']['add']['#theme'] = 'votingapi_actions_add_action';
  $form['actions']['add']['action_type'] = array(
    '#type' => 'select',
    '#options' => _votingapi_actions_list(),
  );
  $form['actions']['add']['add_action'] = array(
    '#type' => 'button',
    '#value' => t('Add action'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

function votingapi_actions_condition_form($condition) {
  $handler = $condition['handler'];
}

function votingapi_actions_action_form($action) {
  $form = array();
  $actions = actions_actions_map(actions_get_all_actions());

  $form['value'] = array(
    '#type' => 'hidden',
    '#value' => $action,
  );
  $form['description'] = array(
    '#value' => $actions[$action]['description'],
  );
  $form['remove'] = array(
    '#type' => 'markup',
    '#value' => '<br />',
  );
  return $form;
}

function theme_votingapi_actions_add_action($form) {
  $output = '<div class="container-inline">';
  $output .= drupal_render($form['action_type']);
  $output .= drupal_render($form['add_action']);
  $output .= '</div>';
  return $output;
}

function theme_votingapi_actions_add_condition($form) {
  $output = '<div class="container-inline">';
  $output .= drupal_render($form['condition_type']);
  $output .= drupal_render($form['add_condition']);
  $output .= '</div>';
  return $output;
}

function _votingapi_actions_list($content_type = 'Node') {
  $options = array();
  foreach (actions_actions_map(actions_get_all_actions()) as $aid => $action) {
    $options[$action['type']][$aid] = $action['description'];
  }
  return $options;
}

function _votingapi_actions_condition_types($content_type = 'node') {
  static $handlers;
  if (empty($handlers)) {
    $handlers = module_invoke_all('votingapi_action_handlers');
  }
  $list = array();
  foreach ($handlers as $key => $data) {
    if ($data['type'] == $content_type || $data['type'] == '*') {
      $list[$key] = $data['name'];
    }
  }
  return $list;
}