<?php

class RateLimitFingerprint {
  public $fingerprint;

  public function __construct($user = NULL) {
    if (!$user) $user = $GLOBALS['user'];

    $this->fingerprint['uid'] = $user->uid;
  }
  public function queryConditions($query, $keys) {
    $conditions = array();
    foreach ($keys as $k) {
      if (isset($this->fingerprint[$k])) {
        $query->condition($k, $this->fingerprint[$k]);
      }
    }
  }
}

class RateLimit {
  protected $fingerprint;
  protected $action_id;
  protected $limits;
  public function __construct($fingerprint, $action_id, $limits = array()) {
    $this->fingerprint = $fingerprint;
    $this->action_id = $action_id;
    $this->limits    = $limits;
  }
  public function checkLimit($interval, $times, $keys) {
    $query = db_select('rate_limit_log')
      ->addExpression('COUNT(id)', 'times')
      ->condition('timestamp', time()-$interval, '>=');
    $this->fingerprint->queryConditions($query, $keys);

    $times = 0;
    if ($row = $query->execute()->fetch()) {
      $times = $row->times;
    }

    return $this->times > $times;
  }

  public function log() {
    db_insert('rate_limit_log')
    ->fields(array(
      'action_id' => $this->action_id,
      'timestamp' => time(),
    ) + $this->fingerprint->fingerprint)->exectue();
  }

  public function checkIn($limits = NULL) {
    if (!$limits) {
      $limits = $this->limits;
    }
    foreach ($limits as $limit) {
      list($interval, $times, $keys) = $limit;
      if (!$this->checkLimit($interval, $times, $keys)) {
        return FALSE;
      }
    }
    $this->log();
    return TRUE;
  }

  public static function fromConfig($name, $action_id) {
    $config = RateLimitConfig::load($name);
    $class = $config->fingerprint_class;
    return new RateLimit(new $class(), $action_id, $config->limits);
  }
}

class RateLimitConfig {
  public $name;
  public $fingerprint_class;
  public $limits;
  public function __construct($name, $fingerprint_class, $limits = array()) {
    $this->name = $name;
    $this->fingerprint_class = $fingerprint_class;
    $this->limits = array();
  }
  public function save() {
    drupal_set_variable('rate_limit_config_' . $this->name, $this);
  }
  public static function load($name) {
    return drupal_get_variable('rate_limit_contig_' . $name, NULL);
  }
}
