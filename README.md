# Drupal DoctORM

## What is DoctORM ?
DoctORM is an ORM made for Drupal 7.x. This project was created during my second-year at university.

## How does this work ?
DoctORM was inspired by Doctrine2. You need to config your model through PHP comments (you'll see just below) :
```php
/**
 * @Table(games)
 */
class Game extends DoctORM {

}
```
*Please note that the ```Game``` class extends the ```Doctor``` class !*

To configure attributes and relation-ships, use 
```php
  /**
   * @PrimaryKey
   */
  protected $gid;
```
- ```@PrimaryKey``` indicates that this key is a auto-incremented value. **You can have multiple primary keys.**
```php
  /**
   * @Attribute
   */
  protected $name;
```
- ```@Attribute``` is just an attribute. It **must have** the same name as in the database
```php
  /**
   * @BelongsTo(Editor)
   */
  protected $editor;
```
- ```@BelongsTo(Something)``` indicates that this value is a *Something* object and that it needs to create this object with this id.
```php
/**
   * @HasMany(Game)
   */
  protected $games;
```
- ```@HasMany(Something)``` is the inverse of *BelongsTo*. This attribute will be an array of *Something* objects. To do this, the *Something* class **MUST HAVE** a ```@BelongsTo``` attribute.

```php
/**
   * @Function(CountGames)
   */
  protected $count;
  
  protected function CountGames()
  {
    //Do Something
    return count($this->games);
  }
```
- ```@Function(Something)``` will execute the function specified in the parenthesis and put the return value on this attribute

```php
/**
   * @Type(varchar)
   * @unique
   */
  protected $name;
```
- ```@Type(Something)``` is used for automatic creation of drupal schema. You can specify if this attribute should be unique on the database, and of course the type of this attribute. You can use ```int```, ```varchar``` and ```date``` (only on MySQL/MariaDB).

*Please note the ```@``` before each option !*

## Methods
```php
    Game::debug($game);
```
- ```debug()``` is a static function that can be use on every model that extends ```DoctORM```, and print a well-formatted ```var_dump```.
```php
function mymodule_schema()
{
    $schema['games'] = Game::createSchema();
    return $schema;
}
```
- ```createSchema()``` is a static function that return the array needed for the database creation on your module based on the informations of your model.
**That's it !**

## Contributors

I'm the only on working on this project. 

You can contact me on [my website](www.vincentriva.fr), and follow me on Twitter [@MCpTreast](https://twitter.com/MCpTreast).

## License

This project is under **Creative Commons Licence Attribution (BY)**, that's mean you can :
- Use it for commercial purposes
- Modify it
- Distribute it
- Make derivate works and remixes

if only you write my name (Vincent Riva) somewhere visible by the public.
### Example in HTML
```html
<!-- This project run with DoctORM by Vincent Riva (https://github.com/Treast/drupal-doctorm) -->
```