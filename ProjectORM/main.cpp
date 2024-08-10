#include <iostream>
#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Postgres.h>
#include <string>
#include <Windows.h>

#pragma execution_character_set("utf-8")

class Publisher;
class Book;
class Stock;
class Sale;
class Shop;

class Publisher
{
public:
   std::string name;
   Wt::Dbo::collection<Wt::Dbo::ptr<Book>> books;

   template <typename Action>
   void persist(Action& a)
   {
      Wt::Dbo::field(a, name, "name");
      Wt::Dbo::hasMany(a, books, Wt::Dbo::ManyToOne, "publisher");
   }
};

class Book
{
public:
   std::string title;
   Wt::Dbo::ptr<Publisher> publisher;
   Wt::Dbo::collection<Wt::Dbo::ptr<Stock>> stocks;

   template <typename Action>
   void persist(Action& a)
   {
      Wt::Dbo::field(a, title, "title");
      Wt::Dbo::belongsTo(a, publisher, "publisher");
      Wt::Dbo::hasMany(a, stocks, Wt::Dbo::ManyToOne, "book");
   }
};

class Stock
{
public:
   int count;
   Wt::Dbo::ptr<Book> book;
   Wt::Dbo::ptr<Shop> shop;
   Wt::Dbo::collection<Wt::Dbo::ptr<Sale>> sales;

   template <typename Action>
   void persist(Action& a)
   {
      Wt::Dbo::field(a, count, "count");
      Wt::Dbo::belongsTo(a, book, "book");
      Wt::Dbo::belongsTo(a, shop, "shop");
   }
};

class Sale
{
public:
   double price;
   std::string data_sale;
   int count;
   Wt::Dbo::ptr<Stock> stock;

   template <typename Action>
   void persist(Action& a)
   {
      Wt::Dbo::field(a, price, "price");
      Wt::Dbo::field(a, data_sale, "data_sale");
      Wt::Dbo::field(a, count, "count");
      Wt::Dbo::belongsTo(a, stock, "stock");
   }
};

class Shop
{
public:
   std::string name;
   Wt::Dbo::collection<Wt::Dbo::ptr<Stock>> stocks;

   template <typename Action>
   void persist(Action& a)
   {
      Wt::Dbo::field(a, name, "name");
      Wt::Dbo::hasMany(a, stocks, Wt::Dbo::ManyToOne, "shop");
   }
};

void fill_tables(Wt::Dbo::Session& s)
{
   Wt::Dbo::Transaction t{ s };

   Wt::Dbo::ptr<Publisher> publisher_1 = s.add(std::make_unique<Publisher>());
   publisher_1.modify()->name = "First_num";
   Wt::Dbo::ptr<Publisher> publisher_2 = s.add(std::make_unique<Publisher>());
   publisher_2.modify()->name = "First_book";

   Wt::Dbo::ptr<Book> book_1 = s.add(std::make_unique<Book>());
   Wt::Dbo::ptr<Book> book_2 = s.add(std::make_unique<Book>());
   book_1.modify()->title = "Язык программирования С++";
   book_2.modify()->title = "С++ без страха";
   book_1.modify()->publisher = publisher_1;
   book_2.modify()->publisher = publisher_2;

   Wt::Dbo::ptr<Shop> Shop_1 = s.add(std::make_unique<Shop>());
   Wt::Dbo::ptr<Shop> Shop_2 = s.add(std::make_unique<Shop>());
   Shop_1.modify()->name = "Shop_1";
   Shop_2.modify()->name = "Shop_2";

   Wt::Dbo::ptr<Sale> Sale_1 = s.add(std::make_unique<Sale>());
   Wt::Dbo::ptr<Sale> Sale_2 = s.add(std::make_unique<Sale>());
   Wt::Dbo::ptr<Sale> Sale_3 = s.add(std::make_unique<Sale>());
   Sale_1.modify()->price = 3000.99;
   Sale_2.modify()->price = 4000.00;
   Sale_3.modify()->price = 3100.00;
   Sale_1.modify()->data_sale = "2020-03-03";
   Sale_2.modify()->data_sale = "2005-03-11";
   Sale_3.modify()->data_sale = "2022-01-12";
   Sale_1.modify()->count = 123;
   Sale_2.modify()->count = 233;
   Sale_3.modify()->count = 300;

   Wt::Dbo::ptr<Stock> Stock_1 = s.add(std::make_unique<Stock>());
   Wt::Dbo::ptr<Stock> Stock_2 = s.add(std::make_unique<Stock>());
   Stock_1.modify()->book = book_1;
   Stock_2.modify()->book = book_2;
   Stock_1.modify()->shop = Shop_1;
   Stock_2.modify()->shop = Shop_2;
   Stock_1.modify()->count = 400;
   Stock_2.modify()->count = 400;

   t.commit();
}

void find_publisher(Wt::Dbo::Session& s, const std::string& which_pub)
{
   Wt::Dbo::Transaction t{ s };
   Wt::Dbo::collection<Wt::Dbo::ptr<Publisher>> Publishers = s.find<Publisher>().where("name = ?").bind(which_pub);

   if (Publishers.size() == 0)
   {
      std::cout << "Издатель не найден" << std::endl;
      return;
   }

   Wt::Dbo::ptr<Publisher> publisher = Publishers.front();
   std::cout << "Издатель: " << publisher->name << std::endl;

   for (const auto& book : publisher->books)
   {
      std::cout << "Книги: " << book->title << std::endl;
   
      for (const auto& stocks : book->stocks)
      {
         std::cout << "Магазины: " << stocks->shop->name << std::endl;
      }
   }

   t.commit();
}

int main()
{
   SetConsoleCP(CP_UTF8);
   SetConsoleOutputCP(CP_UTF8);

   try {
      std::string conn_String = "host=localhost "
         "port=5432 "
         "dbname=mydb "
         "user=postgres "
         "password=1209R";

      auto connection = std::make_unique<Wt::Dbo::backend::Postgres>(conn_String);

      Wt::Dbo::Session s;
      s.setConnection(std::move(connection));

      s.mapClass<Publisher>("publisher");
      s.mapClass<Book>("book");
      s.mapClass<Stock>("stock");
      s.mapClass<Sale>("sale");
      s.mapClass<Shop>("shop");

      try {
         s.createTables();
      }
      catch (const std::exception& e)
      {
         std::cout << "Tables exist, skipping..." << std::endl;
      }
      fill_tables(s);

      std::string which_pub;
      std::cout << "Выберите издателя: ";
      std::getline(std::cin, which_pub);

      find_publisher(s, which_pub);
   }
   catch (const std::exception& e)
   {
      std::cout << "Connection error: " << e.what() << std::endl;
   }

   return 0;
}
