<?php

class DoctORM {

  public function __construct($searchId = null, $recursive = TRUE){
    $idField = self::getPropertiesName('PrimaryKey')[0];
    if (is_null($searchId)) {
      $id = db_insert(self::getTableName())
      ->useDefaults(self::getSelectableProperties())
      ->execute();
      $this->$idField = $id;
    } else {
      $query = db_select(self::getTableName(), 'x')
      ->fields('x')
      ->condition($idField, $searchId, '=');
      $object = $query->execute();
      try {
        self::buildObject($object, $recursive);
      } catch (Exception $e) {
        die($e->getMessage());
      }
    }
  }
  public function __get($field) {
    try {
      return $this->$field;
    } catch (Exception $e) {
      return null;
    }
  }

  public function delete(){
    $table = self::getTableName();
    $id = self::getPropertiesName('PrimaryKey')[0];
    $query = db_delete($table)
    ->condition($id, $this->$id)
    ->execute();
  }

  public function __set($field, $value) {
    if ($value != null) {
      if(property_exists(get_class($this), $field)){
        $table = self::getTableName();
        if(in_array($field, self::getSelectableProperties())){
          $id = self::getPropertiesName('PrimaryKey')[0];
          $query = db_update($table)
          ->fields(array($field => $value))
          ->condition($id, $this->$id)
          ->execute();
          $this->$field = $value;
        }
      }else{
        throw new Exception("Unknown variable: ".$field);
      }
    }
  }

  private function buildObject($object, $recursive = TRUE){
    if($object->rowCount() == 1) {
      $object = $object->fetchObject();
      foreach ($object as $key => $value) {
        if(property_exists($this, $key)){
          if (array_key_exists('Attribute', self::getPropertyType($key)) || array_key_exists('PrimaryKey', self::getPropertyType($key))) {
            $this->$key = $value;
          } elseif (array_key_exists('BelongsTo', self::getPropertyType($key)) && !is_null($value)) {
            if ($recursive) {
              $class = self::getPropertyType($key)['BelongsTo'];
              $this->$key = new $class($value);
            } else {
              $this->$key = $value;
            }
          } else {
            $this->$key = null;
          }
        }
      }
      foreach (self::getProperties('HasMany') as $key => $value) {
        $class = self::getPropertyType($key)['HasMany'];
        $field = $class::getHasManyName(get_called_class());
          if ($recursive) {
            $fieldId = self::getPropertiesName('PrimaryKey')[0];
            $id = $this->$fieldId;
            $this->$key = $class::findAll(array($fieldId, $id, '='), FALSE);
          }
          else
            $this->$key = null;
      }
      foreach (self::getProperties('Function') as $key => $value) {
        $function = self::getPropertyType($key)['Function'];
        if(method_exists($this, $function))
          $this->$key = $this->$function();
        else
          throw new Exception("DoctorException - ". get_class($this) ." doesn't have a ". $function ." method.");
      }
    } else {
      throw new Exception("DoctorException - Too much results or zero results. <br>". self::debug($object), 42);  
    }
  }

  private static function getProperties($filters = null){
    $properties = array();
    $reflection = new ReflectionClass(get_called_class());
    foreach($reflection->getProperties() as $property){
      $parse = self::parse($property->getDocComment());
      if((is_null($filters)) ? TRUE : array_key_exists($filters, $parse))
        $properties[$property->getName()] = $parse;
    }
    return $properties;
  }

  private static function getHasManyName($class){
    foreach (self::getProperties('BelongsTo') as $key => $value) {
      if ($value['BelongsTo'] == $class)
        return $key;
    }
    return null;
  }

  private static function getSelectableProperties(){
    return array_diff(self::getPropertiesName(), self::getPropertiesName('HasMany'), self::getPropertiesName('Function'));
  }

  private static function getPropertiesName($filters = null){
    return (count(array_keys(self::getProperties($filters))) > 0) ? array_keys(self::getProperties($filters)) : array();
  }

  private static function getPropertyType($property){
    return (is_null(self::getProperties()[$property])) ? array() : self::getProperties()[$property];
  }

  private static function getTableName(){
    $reflection = new ReflectionClass(get_called_class());
    return self::parse($reflection->getDocComment())['Table'];
  }

  private static function parse($comments){
    $parser = array();
    $lines = explode("\n", $comments);

    foreach($lines as $line){
      preg_match('/\* \@([a-zA-Z0-9]+)\(([a-zA-Z0-9_\-]+)\)/', $line, $matches);
      if(!empty($matches[0])){
        $parser[$matches[1]] = $matches[2];
      }else{
        preg_match('/\* \@([a-zA-Z0-9]+)/', $line, $matches);
        if(!empty($matches[0]))
          $parser[$matches[1]] = 1;
      }
    }
    return $parser;
  }

  public static function findAll($conditions = array(), $recursive = TRUE) {
    $class = get_called_class();
    $query = db_select($class::getTableName(), 'x')
    ->fields('x');
    
    if(count($conditions) > 0) {
      foreach($conditions as $condition){
        if(count($condition) == 3){
          $query->condition($condition[0], $condition[1], $condition[2]);
        }
      }
    }

    $objects = $query->execute();

    $instances = array();
    foreach($objects as $object) {
      foreach ($object as $key => $value) {
        if(property_exists($class, $key)){
          if (array_key_exists('PrimaryKey', self::getPropertyType($key))) {
            $instances[$value] = new $class($value, $recursive);
          }
        }
      }
    }
    return $instances;
  }

  public static function debug($v){
    echo '<pre>'.print_r($v, TRUE).'</pre>';
  }

  public static function findAllByProperty($property, $conditions = array(), $recursive = TRUE){
    if(property_exists(get_called_class(), $property)){
      $property = strtolower($property);
      $objects = self::findAll($conditions, $recursive);
      $array = array();
      $idField = self::getPropertiesName('PrimaryKey')[0];
      foreach($objects as $object)
        $array[$object->$idField] = $object->$property;
      return $array; 
    } else {
      return array();
    }
  }

  public static function createSchema(){
    $fields = array();
    $unique = array();
    $foreign = array();

    foreach(self::getSelectableProperties() as $field){
      if(array_key_exists('PrimaryKey', self::getPropertyType($field))){
        $fields[$field] = array('type' => 'serial', 'unsigned' => TRUE, 'not null' => TRUE);
        $unique[$field] = array($field);
      }else{
        if(isset(self::getPropertyType($field)['Type'])){
          $type = self::getPropertyType($field)['Type'];
          switch($type){
            case 'int': $fields[$field] = array('type' => 'int', 'default' => null); break;
            case 'varchar': $fields[$field] = array('type' => 'varchar', 'length' => 255,'default' => ''); break;
            case 'date': $fields[$field] = array('mysql_type' => 'datetime'); break;
          }
        }
      }
      if(array_key_exists('Unique', self::getPropertyType($field))){
        $unique[$field] = array($field);
      }
      if(array_key_exists('BelongsTo', self::getPropertyType($field))){
        $class = self::getPropertyType($field)['BelongsTo'];
        $name = strtolower(self::getPropertyType($field)['BelongsTo']);
        if(isset($foreign['node_'.$name])){
          $foreign['node_'.$name]['columns'][$field] = $class::getPropertiesName('PrimaryKey');
        }else{
          $foreign['node_'.$name] = array(
            'table' => $class::getTableName(),
            'columns' => array(
              $field => $class::getPropertiesName('PrimaryKey'),
              )
            );
        }
      }
    }

    $schema = array(
      'fields' => $fields,
      'foreign keys' => $foreign,
      'unique keys' => $unique,
      );

    return $schema;
  }
}