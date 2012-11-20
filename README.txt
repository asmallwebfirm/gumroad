-- SUMMARY --

While there's nothing stopping you from just pasting Gumroad share links in your
node bodies or creating a Gumroad share field on your content types, this module
aims to encourage a more structured, integrated approach to selling products
with Gumroad on your Drupal website.

Beyond architectural normalization, it also introduces a standard way to handle
Gumroad's Webhook API in Drupal.


-- FEATURES --

* Creates a "Gumroad product" entity.
* Gumroad products are fieldable by product type.
* Gumroad product types are exportable (Features support)
* Gumroad products have two custom view modes out of the box:
  - Gumroad Link (to only show a link to the Gumroad store; use-cases include
    embedding Gumroad products on other content types via Entity Reference, use
    in Views, etc.)
  - Purchased (to show different fields based on whether or not the product has
    been purchased; this functionality hasn't been built out yet.)
* Gumroad Webhook API handling by product type; three modes out of the box:
  - No special handling: does nothing by default, but could be used for logging.
  - Static URL: provides a single URL for all products of that type. Useful for
    physical products that just need a "thank you" page.
  - Field: select a field attached to the product for dynamic routing. Useful if
    the product is a downloadable file (currently supports Link and File fields)


-- INSTALLATION --

* Ensure Entity API is already installed (http://www.drupal.org/project/entity).
* Install and enable this module.
* Add/configure a Gumroad product type (admin/structure/gumroad-product-types)
* Start adding products (gumroad/add)


-- USAGE --

This module is more of a framework and supports a number of different use-cases.

- GUMROAD PRODUCT ENTITIES AS CONTENT -
In this scenario, you have a Gumroad product entity for every product you have
listed in Gumroad. Each product in Gumroad points to its Gumroad entity within
Drupal.

You'd likely treat your Gumroad product entities as content, creating Views to
represent your store/catalogue, adding fields showing or describing the product.

If your products are downloadable, you'll likely enable field-based Webhook
processing, allowing each product to have a file or link associated with it.
After the user completes a transaction, he/she will be redirected to the file or
link specified on the product.

Product | Gumroad URL | Product "page" on Drupal | Type     | Webhook
----------------------------------------------------------------------------
Foo     | gum.co/foo  | example.com/gumroad/1    | download | field_download
Bar     | gum.co/bar  | example.com/gumroad/2    | download | field_download
Baz     | gum.co/baz  | example.com/gumroad/3    | download | field_download


- GUMROAD PRODUCT ENTITIES FOR FIELD REFERENCES -
In this scenario, you may represent products in Drupal in some other way (for
example, a "Product" content type). Each product may reference one or several
Gumroad product entities using the Entity Reference module. You may not even
have a specific Product entity, but just want to add Gumroad purchase links to
certain types of content, taxonomies, etc.

If you had one, your store/catalogue page would probably be a View of your
"Product" content type, using the "Gumroad Link" view mode to display "buy"
links for each item. If you didn't have a store/catalogue page, you'd likely
just use the same view mode on the host entity to display a "buy" link for a
particular piece of content.

In this case, you might not even apply fields to your Gumroad products. If you
do, it's only to alter how the "Gumroad Link" view mode works, or if you're
using Webhook processing, to dynamically direct to a specified file or link.

This could be particularly useful if you have Gumroad products that are more or
less the same thing, but you're forced to break them up into separate products
(rather than using variations) because the price differs based on variation.
For example, an XXL t-shirt that's slightly more expensive, or one album for a
band that's available as a CD, Vinyl, or just a digital download.

Product | Gumroad URL | Product "page" on Drupal | Type     | Webhook
----------------------------------------------------------------------------
Foo (S) | gum.co/foos | example.com/shirts/foo   | shirt    | static url
Foo (M) | gum.co/foom | example.com/shirts/foo   | shirt    | static url
Bar     | gum.co/bar  | example.com/albums/bar   | download | field_download

You might also go this route if you only have a few Gumroad products and don't
need a store/catalogue, or at least don't want to manage it as a View of Gumroad
entities.
